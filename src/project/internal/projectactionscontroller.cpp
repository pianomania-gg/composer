/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "projectactionscontroller.h"

#include <QBuffer>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QSaveFile>
#include <QTemporaryDir>
#include <QStandardPaths>
#include <QRegularExpression>
#include <QSettings>
#include "composerpackage.h"
#include "composerbackgrounddialog.h"
#include "composersongdetails.h"
#include "composersession.h"
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QEventLoop>
#include <QFileInfo>
#include <QRadioButton>
#include <QTemporaryFile>
#include <QUrl>
#include <QUrlQuery>
#include <QTimer>
#include <QVBoxLayout>

#include "async/async.h"
#include "defer.h"
#include "translation.h"

#include "cloud/clouderrors.h"
#include "cloud/qml/Muse/Cloud/enums.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/score.h"
#include "engraving/infrastructure/mscio.h"
#include "engraving/engravingerrors.h"

#include "pianomaniaexport.h"
#include "notationcompatibility.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QXmlStreamReader>
#include "notation/inotationinteraction.h"
#include "projecterrors.h"
#include "projectextensionpoints.h"

#include "log.h"

using namespace mu;
using namespace mu::project;
using namespace mu::notation;
using namespace muse;
using namespace muse::actions;

static const muse::Uri NOTATION_PAGE_URI("musescore://notation");
static const muse::Uri HOME_PAGE_URI("musescore://home");
static const muse::Uri NEW_SCORE_URI("musescore://project/newscore");
static const muse::Uri PROJECT_PROPERTIES_URI("musescore://project/properties");

static constexpr int RETRY_SAVE_BTN_ID = int(IInteractive::Button::CustomButton);
static constexpr int SAVE_AS_BTN_ID    = RETRY_SAVE_BTN_ID + 1;
namespace {
struct CompatibilityNavigation {
    QHash<QString, int> measureOrdinals;
    std::function<bool(int, bool)> navigate;
};

QStringList compatibilityMeasureSignature(mu::engraving::Score* score, bool includeIdentity)
{
    QStringList signature;
    if (!score) return signature;
    for (auto* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        const QString identity = includeIdentity && measure->eid().isValid()
            ? QString::fromStdString(measure->eid().toStdString()) : QString();
        signature.append(identity + QString("|%1/%2|%3/%4|%5|%6")
            .arg(measure->tick().numerator()).arg(measure->tick().denominator())
            .arg(measure->ticks().numerator()).arg(measure->ticks().denominator())
            .arg(measure->no()).arg(measure->irregular() ? 1 : 0));
    }
    return signature;
}

CompatibilityNavigation compatibilityNavigation(const std::shared_ptr<mu::context::IGlobalContext>& context,
                                                const mu::notation::INotationPtr& notation,
                                                const QByteArray& exportedMei = {},
                                                mu::engraving::Score* exportedScore = nullptr)
{
    CompatibilityNavigation result;
    if (!context || !notation || !notation->elements() || !notation->elements()->msScore()) return result;
    auto* score = notation->elements()->msScore();
    const QStringList signature = compatibilityMeasureSignature(score, true);
    std::vector<mu::engraving::Measure*> measures;
    for (auto* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        if (!measure->eid().isValid()) return {};
        const int ordinal = int(measures.size());
        result.measureOrdinals.insert(QString::fromStdString(measure->eid().toStdString()), ordinal);
        measures.push_back(measure);
    }
    if (measures.empty()) return {};
    if (!exportedMei.isEmpty()) {
        // Validate the written snapshot sequence before associating canonical measure IDs with its ordinals.
        if (!exportedScore || compatibilityMeasureSignature(exportedScore, false) != compatibilityMeasureSignature(score, false)) return {};
        std::vector<mu::engraving::Measure*> exportedMeasures;
        for (auto* measure = exportedScore->firstMeasure(); measure; measure = measure->nextMeasure()) exportedMeasures.push_back(measure);
        QXmlStreamReader reader(exportedMei);
        QSet<QString> seen;
        int ordinal = 0;
        while (!reader.atEnd()) {
            reader.readNext();
            if (!reader.isStartElement() || reader.name() != "measure") continue;
            if (reader.namespaceUri() != "http://www.music-encoding.org/ns/mei" || ordinal >= int(exportedMeasures.size())) return {};
            const auto attributes = reader.attributes();
            const QString identity = attributes.value("http://www.w3.org/XML/1998/namespace", "id").toString();
            if (identity.isEmpty() || seen.contains(identity)) return {};
            auto* measure = exportedMeasures[ordinal];
            if (attributes.hasAttribute("n")) {
                if (attributes.value("n") != QString::number(measure->no() + 1)) return {};
            } else if (!measure->irregular()) return {};
            seen.insert(identity);
            result.measureOrdinals.insert(identity, ordinal);
            if (measure->eid().isValid()) result.measureOrdinals.insert(QString::fromStdString(measure->eid().toStdString()), ordinal);
            ++ordinal;
        }
        if (reader.hasError() || ordinal != int(measures.size())) return {};
    }
    std::weak_ptr<mu::context::IGlobalContext> weakContext = context;
    std::weak_ptr<mu::notation::INotation> weakNotation = notation;
    result.navigate = [weakContext, weakNotation, signature](int ordinal, bool perform) {
        const auto context = weakContext.lock();
        const auto notation = weakNotation.lock();
        if (!context || !notation || context->currentNotation() != notation || !notation->isOpen() || !notation->elements()) return false;
        auto* score = notation->elements()->msScore();
        if (!score || compatibilityMeasureSignature(score, true) != signature || ordinal < 0) return false;
        auto* measure = score->firstMeasure();
        for (int index = 0; measure && index < ordinal; ++index) measure = measure->nextMeasure();
        const auto interaction = notation->interaction();
        if (!measure || !interaction) return false;
        if (perform) {
            interaction->select({measure}, mu::notation::SelectType::REPLACE);
            interaction->showItem(measure);
        }
        return true;
    };
    return result;
}
}


void ProjectActionsController::init()
{
    dispatcher()->reg(this, "file-new", this, &ProjectActionsController::newProject);
    dispatcher()->reg(this, "file-open", this, &ProjectActionsController::openProject);

    dispatcher()->reg(this, "file-close", [this]() {
        auto anyInstanceWithoutProject = multiwindowsProvider()->isHasWindowWithoutProject();
        bool ok = closeOpenedProject();
        if (ok && anyInstanceWithoutProject) {
            //! NOTE: we need to call `quit` in the next event loop due to controlling the lifecycle of this method
            async::Async::call(this, [this]() {
                dispatcher()->dispatch("quit", ActionData::make_arg1<bool>(false));
            });
            multiwindowsProvider()->activateWindowWithoutProject();
        }
    });

    dispatcher()->reg(this, "file-save", [this]() { saveProject(SaveMode::Save); });
    dispatcher()->reg(this, "file-save-as", [this]() { saveProject(SaveMode::SaveAs); });
    dispatcher()->reg(this, "file-save-a-copy", [this]() { saveProject(SaveMode::SaveCopy); });
    dispatcher()->reg(this, "file-save-selection", [this]() { saveProject(SaveMode::SaveSelection, SaveLocationType::Local); });
    dispatcher()->reg(this, "file-save-at", [this](const ActionData& args) { saveProjectAt(args); });

    dispatcher()->reg(this, "file-export", this, &ProjectActionsController::exportScore);
    dispatcher()->reg(this, "file-export-composer", this, &ProjectActionsController::exportComposer);
    dispatcher()->reg(this, "composer-song-details", [this]() { editComposerSongDetails(); });
    dispatcher()->reg(this, "composer-account", []() { composer::showAccount(QApplication::activeWindow()); });
#ifndef PIANOMANIA_COMPOSER_PRODUCTION
    dispatcher()->reg(this, "file-export-pianomania", this, &ProjectActionsController::exportPianomania);
#endif

    dispatcher()->reg(this, "print", this, &ProjectActionsController::printScore);

    dispatcher()->reg(this, "clear-recent", this, &ProjectActionsController::clearRecentScores);

    dispatcher()->reg(this, "continue-last-session", this, &ProjectActionsController::continueLastSession);

    dispatcher()->reg(this, "project-properties", this, &ProjectActionsController::openProjectProperties);
}

INotationProjectPtr ProjectActionsController::currentNotationProject() const
{
    return globalContext()->currentProject();
}

IMasterNotationPtr ProjectActionsController::currentMasterNotation() const
{
    return currentNotationProject() ? currentNotationProject()->masterNotation() : nullptr;
}

INotationPtr ProjectActionsController::currentNotation() const
{
    return currentMasterNotation() ? currentMasterNotation()->notation() : nullptr;
}

INotationInteractionPtr ProjectActionsController::currentInteraction() const
{
    return currentNotation() ? currentNotation()->interaction() : nullptr;
}

INotationSelectionPtr ProjectActionsController::currentNotationSelection() const
{
    return currentNotation() ? currentInteraction()->selection() : nullptr;
}

bool ProjectActionsController::canReceiveAction(const ActionCode& code) const
{
#ifdef PIANOMANIA_COMPOSER_PRODUCTION
    if (code == "file-export-pianomania") return false;
#endif
    if (!currentNotationProject()) {
        static const std::unordered_set<ActionCode> DONT_REQUIRE_OPEN_PROJECT {
            "file-new",
            "file-open",
            "continue-last-session",
            "clear-recent",
            "composer-account",
        };

        return muse::contains(DONT_REQUIRE_OPEN_PROJECT, code);
    }

    return true;
}

bool ProjectActionsController::isUrlSupported(const QUrl& url) const
{
    if (url.isLocalFile()) {
        return isFileSupported(muse::io::path_t(url));
    }

    return false;
}

bool ProjectActionsController::isFileSupported(const muse::io::path_t& path) const
{
    std::string suffix = io::suffix(path);
    if (engraving::isMuseScoreFile(suffix)) {
        return true;
    }

    if (readers()->reader(suffix)) {
        return true;
    }

    return false;
}

void ProjectActionsController::openProject(const ActionData& args)
{
    QUrl url = !args.empty() ? args.arg<QUrl>(0) : QUrl();
    QString displayNameOverride = args.count() >= 2 ? args.arg<QString>(1) : QString();

    Ret ret = openProject(ProjectFile(url, displayNameOverride));
    if (!ret) {
        LOGE() << ret.toString();
    }
}

Ret ProjectActionsController::openProject(const ProjectFile& file)
{
    LOGI() << "Try open project: url = " << file.url.toString() << ", displayNameOverride = " << file.displayNameOverride;

    if (file.isNull()) {
        auto promise = selectScoreOpeningFile();
        promise.onResolve(this, [this](const io::path_t& askedPath) {
            if (askedPath.empty()) {
                return;
            }

            configuration()->setLastOpenedProjectsPath(io::dirpath(askedPath));

            openProject(askedPath);
        });

        return muse::make_ok();
    }

    if (file.url.isLocalFile()) {
        return openProject(file.path(), file.displayNameOverride);
    }

    return make_ret(Err::UnsupportedUrl);
}

Ret ProjectActionsController::openProject(const muse::io::path_t& givenPath, const QString& displayNameOverride)
{
    //! NOTE This method is synchronous,
    //! but inside `multiwindowsProvider` there can be an event loop
    //! to wait for the responses from other instances, accordingly,
    //! the events (like user click) can be executed and this method can be called several times,
    //! before the end of the current call.
    //! So we ignore all subsequent calls until the current one completes.
    if (m_isProjectProcessing) {
        return make_ret(Ret::Code::InternalError);
    }
    m_isProjectProcessing = true;

    DEFER {
        m_isProjectProcessing = false;
    };

    //! Step 1. Take absolute path
    muse::io::path_t actualPath = fileSystem()->absoluteFilePath(givenPath);
    if (actualPath.empty()) {
        // We assume that a valid path has been specified to this method
        return make_ret(Ret::Code::UnknownError);
    }

    //! Step 2. If the project is already open in the current window, then just switch to showing the notation
    if (isProjectOpened(actualPath)) {
        return doFinishOpenProject();
    }

    //! Step 3. Check, if the project already opened in another window, then activate the window with the project
    if (multiwindowsProvider()->isProjectAlreadyOpened(actualPath)) {
        multiwindowsProvider()->activateWindowWithProject(actualPath);
        return make_ret(Ret::Code::Ok);
    }

    //! Step 4. Check, if a any project is already open in the current window,
    //! then create a new instance
    if (globalContext()->currentProject()) {
        QStringList args;
        args << actualPath.toQString();

        if (!displayNameOverride.isEmpty()) {
            args << "--score-display-name-override" << displayNameOverride;
        }

        multiwindowsProvider()->openNewWindow(args);
        return make_ret(Ret::Code::Ok);
    }

    // Composer opens the selected disk file without checking a cloud copy.
    return doOpenProject(actualPath);
}

RetVal<INotationProjectPtr> ProjectActionsController::loadProject(const muse::io::path_t& filePath)
{
    TRACEFUNC;

    const auto project = projectCreator()->newProject(iocContext());
    IF_ASSERT_FAILED(project) {
        return make_ret(Ret::Code::InternalError);
    }

    const bool hasUnsavedChanges = projectAutoSaver()->projectHasUnsavedChanges(filePath);

    const muse::io::path_t loadPath = hasUnsavedChanges ? projectAutoSaver()->projectAutoSavePath(filePath) : filePath;
    const std::string format = io::suffix(filePath);

    if (Ret result = loadWithFallback(project, loadPath, format); !result) {
        return result;
    }

    if (hasUnsavedChanges) {
        //! NOTE: redirect the project to the original file path
        project->setPath(filePath);

        project->markAsUnsaved();
    }

    if (projectAutoSaver()->isAutosaveOfNewlyCreatedProject(filePath)) {
        project->markAsNewlyCreated();
    }

    return RetVal<INotationProjectPtr>::make_ok(project);
}

Ret ProjectActionsController::loadWithFallback(const std::shared_ptr<INotationProject>& project,
                                               const muse::io::path_t& loadPath,
                                               const std::string& format)
{
    Ret result = project->load(loadPath, OpenParams(), format);

    if (result || result.code() == static_cast<int>(Ret::Code::Cancel)) {
        return result;
    }

    bool forceLoad = shouldRetryLoadAfterError(result, loadPath);
    if (forceLoad) {
        OpenParams params;
        params.forceMode = forceLoad;
        result = project->load(loadPath, params, format);
    }

    return result;
}

Ret ProjectActionsController::doOpenProject(const muse::io::path_t& filePath)
{
    TRACEFUNC;

    RetVal<INotationProjectPtr> rv = loadProject(filePath);
    if (!rv.ret) {
        return rv.ret;
    }

    INotationProjectPtr project = rv.val;

    bool isNewlyCreated = projectAutoSaver()->isAutosaveOfNewlyCreatedProject(filePath);
    if (!isNewlyCreated) {
        recentFilesController()->prependRecentFile(makeRecentFile(project));
    }

    globalContext()->setCurrentProject(project);

    return doFinishOpenProject();
}

Ret ProjectActionsController::doFinishOpenProject()
{
    extensionsProvider()->performPointAsync(EXEC_ONPOST_PROJECT_OPENED);

    Ret opened = openPageIfNeed(NOTATION_PAGE_URI);
    auto notation = currentNotation();
    if (opened && notation && notation->elements()->msScore()) {
        auto inventory = notationcompatibility::inventoryScore(notation->elements()->msScore(),
                                                               currentNotationProject()->path().toQString());
        const auto navigation = compatibilityNavigation(globalContext(), notation);
        if (notationcompatibility::SummaryModel::instance()->setReport(inventory.report, navigation.measureOrdinals, navigation.navigate)) {
            static const Uri compatibilityUri("musescore://project/notation-compatibility");
            if (!interactive()->isOpened(compatibilityUri).val) {
                UriQuery query(compatibilityUri);
                query.addParam("modal", Val(false));
                interactive()->open(query);
            }
        }
    }
    return opened;
}

const ProjectBeingDownloaded& ProjectActionsController::projectBeingDownloaded() const
{
    return m_projectBeingDownloaded;
}

muse::async::Notification ProjectActionsController::projectBeingDownloadedChanged() const
{
    return m_projectBeingDownloadedChanged;
}

Ret ProjectActionsController::openPageIfNeed(Uri pageUri)
{
    if (!interactive()->isOpened(pageUri).val) {
        interactive()->open(pageUri);
    }
    return make_ret(Ret::Code::Ok);
}

bool ProjectActionsController::isProjectOpened(const muse::io::path_t& scorePath) const
{
    auto project = globalContext()->currentProject();
    if (!project) {
        return false;
    }

    LOGD() << "project->path: " << project->path() << ", check path: " << scorePath;
    if (project->path() == scorePath) {
        return true;
    }

    return false;
}

bool ProjectActionsController::isAnyProjectOpened() const
{
    auto project = globalContext()->currentProject();
    if (project) {
        return true;
    }
    return false;
}

void ProjectActionsController::newProject()
{
    //! NOTE This method is synchronous,
    //! but inside `multiwindowsProvider` there can be an event loop
    //! to wait for the responses from other instances, accordingly,
    //! the events (like user click) can be executed and this method can be called several times,
    //! before the end of the current call.
    //! So we ignore all subsequent calls until the current one completes.
    if (m_isProjectProcessing) {
        return;
    }
    m_isProjectProcessing = true;

    DEFER {
        m_isProjectProcessing = false;
    };

    if (globalContext()->currentProject()) {
        if (multiwindowsProvider()->isHasWindowWithoutProject()) {
            multiwindowsProvider()->activateWindowWithoutProject({ "file-new" });
            return;
        }
        QStringList args;
        args << "--session-type" << "start-with-new";
        multiwindowsProvider()->openNewWindow(args);
        return;
    }

    auto promise = interactive()->open(NEW_SCORE_URI);
    promise.onResolve(this, [this](const Val&) {
        extensionsProvider()->performPointAsync(EXEC_ONPOST_PROJECT_CREATED);

        Ret ret = doFinishOpenProject();

        if (!ret) {
            LOGE() << ret.toString();
        }
    });
}

bool ProjectActionsController::closeOpenedProject(bool goToHome)
{
    if (m_isProjectClosing) {
        return false;
    }

    if (m_isProjectSaving) {
        return false;
    }

    m_isProjectClosing = true;
    DEFER {
        m_isProjectClosing = false;
    };

    INotationProjectPtr project = currentNotationProject();
    if (!project) {
        return true;
    }

    if (playbackController()->isPlaying()) {
        playbackController()->reset();
    }

    bool result = true;

    if (project->needSave().val) {
        IInteractive::Button btn = askAboutSavingScore(project);

        if (btn == IInteractive::Button::Cancel) {
            result = false;
        } else if (btn == IInteractive::Button::Save) {
            result = saveProject();
        } else if (btn == IInteractive::Button::DontSave) {
            result = true;
        }
    }

    if (result) {
        interactive()->closeAllDialogsSync();
        globalContext()->setCurrentProject(nullptr);

        if (goToHome) {
            Ret ret = openPageIfNeed(HOME_PAGE_URI);
            if (!ret) {
                LOGE() << ret.toString();
            }
        }
    }

    return result;
}

IInteractive::Button ProjectActionsController::askAboutSavingScore(INotationProjectPtr project)
{
    std::string title = muse::qtrc("project", "Do you want to save changes to the score “%1” before closing?")
                        .arg(project->displayName()).toStdString();

    std::string body = muse::trc("project", "Your changes will be lost if you don’t save them.");

    IInteractive::Result result = interactive()->warningSync(title, body, {
        IInteractive::Button::DontSave,
        IInteractive::Button::Cancel,
        IInteractive::Button::Save
    }, IInteractive::Button::Save);

    return result.standardButton();
}

Ret ProjectActionsController::canSaveProject() const
{
    auto project = currentNotationProject();
    if (!project) {
        LOGW() << "no current project";
        return make_ret(Err::NoProjectError);
    }

    return project->canSave();
}

bool ProjectActionsController::saveProject(const muse::io::path_t& path)
{
    if (!path.empty()) {
        if (m_isProjectSaving) {
            return false;
        }

        m_isProjectSaving = true;
        DEFER {
            m_isProjectSaving = false;
        };

        return saveProjectAt(SaveLocation(SaveLocationType::Local, path));
    }

    return saveProject(SaveMode::Save);
}

bool ProjectActionsController::saveProject(SaveMode saveMode, SaveLocationType saveLocationType, bool force)
{
    if (m_isProjectSaving) {
        return false;
    }

    m_isProjectSaving = true;
    DEFER {
        m_isProjectSaving = false;
    };

    INotationProjectPtr project = currentNotationProject();

    if (project->isCloudProject()) {
        project->setCloudInfo(CloudProjectInfo());
        saveMode = SaveMode::SaveAs;
    }
    if (saveMode == SaveMode::Save && !project->isNewlyCreated()) {
        return saveProjectAt(SaveLocation(SaveLocationType::Local), saveMode, force);
    }
    saveLocationType = SaveLocationType::Local;

    RetVal<SaveLocation> response = openSaveProjectScenario()->askSaveLocation(project, saveMode, saveLocationType);
    if (!response.ret) {
        LOGE() << response.ret.toString();
        return false;
    }

    return saveProjectAt(response.val, saveMode, force);
}

void ProjectActionsController::saveProjectAt(const muse::actions::ActionData& args)
{
    io::path_t path = !args.empty() ? args.arg<io::path_t>(0) : io::path_t();
    if (!path.empty()) {
        saveProjectAt(SaveLocation(path));
    }
}

bool ProjectActionsController::saveProjectAt(const SaveLocation& location, SaveMode saveMode, bool force)
{
    INotationInteractionPtr interaction = currentInteraction();
    if (interaction && interaction->isTextEditingStarted()) {
        interaction->endEditText();
    }

    if (!force) {
        Ret ret = canSaveProject();
        if (!ret) {
            ret = askIfUserAgreesToSaveProjectWithErrors(ret, location);
            if (!ret) {
                return ret;
            }
        }
    }

    if (location.isLocal()) {
        return saveProjectLocally(location.localPath(), saveMode);
    }

    if (location.isCloud()) {
        return false;
    }

    return false;
}

bool ProjectActionsController::saveProjectLocally(const muse::io::path_t& filePath, SaveMode saveMode, bool createBackup)
{
    INotationProjectPtr project = currentNotationProject();
    if (!project) {
        return false;
    }

    Ret ret = make_ok();
    if (saveMode == SaveMode::Save) {
        ret = extensionsProvider()->performPoint(EXEC_ONPRE_PROJECT_SAVE);
    }

    if (ret) {
        ret = project->save(filePath, saveMode, createBackup);
    }

    if (!ret) {
        LOGE() << ret.toString();
        if (ret.code() != (int)Err::CorruptionUponSavingError) {
            warnScoreCouldnotBeSaved(ret);
        } else {
            switch (warnScoreHasBecomeCorruptedAfterSave(ret)) {
            case RETRY_SAVE_BTN_ID:
                async::Async::call(this, [this, filePath, saveMode]() {
                    // Retry the save. Do not create a backup this time because the target file has been corrupted
                    // already. Creating a backup file of a corrupted file now makes no sense and will corrupt
                    // the healthy backup file created on the first save attempt.
                    saveProjectLocally(filePath, saveMode, false /*createBackup*/);
                });
                break;

            case SAVE_AS_BTN_ID:
                async::Async::call(this, [this]() {
                    saveProject(SaveMode::SaveAs);
                });
                break;
            }
        }
        return false;
    }

    if (saveMode == SaveMode::Save) {
        ret = extensionsProvider()->performPoint(EXEC_ONPOST_PROJECT_SAVED);
    }

    recentFilesController()->prependRecentFile(makeRecentFile(project));
    return true;
}

bool ProjectActionsController::askIfUserAgreesToSaveProjectWithErrors(const Ret& ret, const SaveLocation& location)
{
    switch (static_cast<Err>(ret.code())) {
    case Err::NoPartsError:
        warnScoreCouldnotBeSaved(muse::trc("project/save", "Please add at least one instrument to enable saving."));
        return false;
    case Err::CorruptionUponOpenningError:
        return askIfUserAgreesToSaveCorruptedScoreUponOpenning(location, ret.text());
    case Err::CorruptionError: {
        auto project = currentNotationProject();
        return askIfUserAgreesToSaveCorruptedScore(location, ret.text(), project->isNewlyCreated());
    }
    default:
        return false;
    }
}

bool ProjectActionsController::askIfUserAgreesToSaveCorruptedScore(const SaveLocation& location, const std::string& errorText,
                                                                   bool newlyCreated)
{
    switch (location.type) {
    case SaveLocationType::Cloud:
        return false;
    case SaveLocationType::Local:
        return askIfUserAgreesToSaveCorruptedScoreLocally(errorText, !newlyCreated);
    case SaveLocationType::Undefined: // fallthrough
    default:
        return false;
    }
}

bool ProjectActionsController::askIfUserAgreesToSaveCorruptedScoreLocally(const std::string& errorText,
                                                                          bool canRevert)
{
    std::string title = muse::trc("project", "This score has become corrupted and contains errors");

    IInteractive::Text text;
    text.text = !canRevert
                ? muse::trc("project", "You can continue saving it locally, although the file may become unusable. "
                                       "You can try to fix the errors manually, or get help for this issue on Pianomania.")
                : muse::trc("project", "You can continue saving it locally, although the file may become unusable. "
                                       "To preserve your score, revert to the last saved version, or fix the errors manually. "
                                       "You can also get help for this issue on Pianomania.");
    text.detailedText = errorText;

    IInteractive::ButtonDatas buttons;
    buttons.push_back(interactive()->buttonData(IInteractive::Button::Cancel));

    IInteractive::ButtonData saveAnywayBtn(IInteractive::Button::CustomButton, muse::trc("project", "Save anyway"), !canRevert /*accent*/);
    buttons.push_back(saveAnywayBtn);

    int defaultBtn = saveAnywayBtn.btn;

    IInteractive::ButtonData revertToLastSavedBtn(saveAnywayBtn.btn + 1, muse::trc("project", "Revert to last saved"),
                                                  true /*accent*/);
    if (canRevert) {
        buttons.push_back(revertToLastSavedBtn);
        defaultBtn = revertToLastSavedBtn.btn;
    }

    int btn = interactive()->errorSync(title, text, buttons, defaultBtn).button();

    if (btn == revertToLastSavedBtn.btn) {
        revertCorruptedScoreToLastSaved();
    }

    return btn == saveAnywayBtn.btn;
}

bool ProjectActionsController::askIfUserAgreesToSaveCorruptedScoreUponOpenning(const SaveLocation& location, const std::string& errorText)
{
    switch (location.type) {
    case SaveLocationType::Cloud:
        return false;
    case SaveLocationType::Local:
        return askIfUserAgreesToSaveCorruptedScoreLocally(errorText, false /*canRevert*/);
    case SaveLocationType::Undefined: // fallthrough
    default:
        return false;
    }
}

void ProjectActionsController::showErrCorruptedScoreCannotBeSaved(const SaveLocation& location, const std::string& errorText)
{
    std::string title = location.isLocal()
                        ? muse::trc("project", "Your score cannot be saved")
                        : muse::trc("project", "Your score cannot be uploaded to the cloud");

    IInteractive::Text text;
    text.text = muse::trc("project", "This score is corrupted. You can get help for this issue on Pianomania.");
    text.detailedText = errorText;

    IInteractive::ButtonData getHelpBtn(IInteractive::Button::CustomButton, muse::trc("project", "Get help"));

    interactive()->error(title, text, {
        getHelpBtn,
        interactive()->buttonData(IInteractive::Button::Ok)
    }).onResolve(this, [this, getHelpBtn](const IInteractive::Result& res) {
        if (res.isButton(getHelpBtn.btn)) {
            interactive()->openUrl(configuration()->supportForumUrl());
        }
    });
}

void ProjectActionsController::warnScoreCouldnotBeSaved(const Ret& ret)
{
    std::string message = ret.text();
    if (message.empty()) {
        message = muse::trc("project/save", "An unknown error occurred while saving this file.");
    }

    warnScoreCouldnotBeSaved(message);
}

void ProjectActionsController::warnScoreCouldnotBeSaved(const std::string& errorText)
{
    interactive()->warning(muse::trc("project/save", "Your score could not be saved"), errorText);
}

int ProjectActionsController::warnScoreHasBecomeCorruptedAfterSave(const Ret& ret)
{
    const QString errDetailsMessage = QString::fromStdString(ret.toString()).toHtmlEscaped();

    const QString supportForumLink = String("<a href=\"%1\" style=\"text-decoration: none\">Pianomania</a>")
                                     .arg(configuration()->supportForumUrl().toString());

    const std::string title = muse::trc("project/save", "An error occurred while saving your score");

    const std::string body = muse::qtrc("project/save",
                                        "To preserve your score, try saving it again. "
                                        "If this message still appears, please save your score as new copy. "
                                        "You can also get help for this issue on %1.<br/><br/>"
                                        "Error details (please cite when asking for support): %2")
                             .arg(supportForumLink, errDetailsMessage)
                             .toStdString();

    IInteractive::ButtonDatas buttons;

    IInteractive::ButtonData saveAsBtn(SAVE_AS_BTN_ID, muse::trc("project/save", "Save as…"));
    saveAsBtn.role = IInteractive::ButtonRole::ContinueRole;
    buttons.push_back(saveAsBtn);

    IInteractive::ButtonData retryBtn(RETRY_SAVE_BTN_ID, muse::trc("project", "Try again"), true /*accent*/);
    retryBtn.role = IInteractive::ButtonRole::ContinueRole;
    buttons.push_back(retryBtn);

    IInteractive::ButtonData cancelBtn = interactive()->buttonData(IInteractive::Button::Cancel);
    buttons.push_back(cancelBtn);

    return interactive()->errorSync(title, IInteractive::Text(body, IInteractive::TextFormat::RichText),
                                    buttons, retryBtn.btn).button();
}

void ProjectActionsController::revertCorruptedScoreToLastSaved()
{
    TRACEFUNC;

    std::string title = muse::trc("project", "Revert to last saved?");
    std::string body = muse::trc("project", "Your changes will be lost. This action cannot be undone.");

    auto promise = interactive()->warning(title, body, {
        { IInteractive::Button::No, IInteractive::Button::Yes }
    }, IInteractive::Button::Yes, IInteractive::Option::WithIcon);

    promise.onResolve(this, [this](const IInteractive::Result& res) {
        if (res.isButton(IInteractive::Button::No)) {
            return;
        }

        auto currentProject = currentNotationProject();
        muse::io::path_t filePath = currentProject->path();

        bool hasUnsavedChanges = projectAutoSaver()->projectHasUnsavedChanges(filePath);
        if (hasUnsavedChanges) {
            muse::io::path_t autoSavePath = projectAutoSaver()->projectAutoSavePath(filePath);
            fileSystem()->remove(autoSavePath);
        }

        Ret ret = doOpenProject(filePath);
        if (!ret) {
            LOGE() << ret.toString();
        }
    });
}

RecentFile ProjectActionsController::makeRecentFile(INotationProjectPtr project)
{
    RecentFile file;
    file.path = project->path();

    if (project->isCloudProject()) {
        file.displayNameOverride = project->cloudInfo().name;
    }

    return file;
}

bool ProjectActionsController::shouldRetryLoadAfterError(const Ret& ret, const muse::io::path_t& filepath)
{
    if (ret) {
        return true;
    }

    switch (static_cast<engraving::Err>(ret.code())) {
    case engraving::Err::FileTooOld:
    case engraving::Err::FileOld300Format:
        return askIfUserAgreesToOpenProjectWithIncompatibleVersion(
            muse::trc("project", "This score uses an older unsupported file format. Opening it may lose score data."));
    case engraving::Err::FileTooNew:
        warnFileTooNew(filepath);
        return configuration()->disableVersionChecking();
    case engraving::Err::FileCorrupted:
        return askIfUserAgreesToOpenCorruptedProject(io::filename(filepath).toString(), ret.text());
    case engraving::Err::FileCriticallyCorrupted:
        warnProjectCriticallyCorrupted(io::filename(filepath).toString(), ret.text());
        return false;
    default:
        warnProjectCannotBeOpened(ret, filepath);
        break;
    }

    return false;
}

bool ProjectActionsController::askIfUserAgreesToOpenProjectWithIncompatibleVersion(const std::string& errorText)
{
    IInteractive::ButtonData openAnywayBtn(IInteractive::Button::CustomButton, muse::trc("project", "Open anyway"), true /*accent*/);

    int btn = interactive()->warningSync(errorText, "", {
        interactive()->buttonData(IInteractive::Button::Cancel),
        openAnywayBtn
    }, openAnywayBtn.btn).button();

    return btn == openAnywayBtn.btn;
}

void ProjectActionsController::warnFileTooNew(const muse::io::path_t& filepath)
{
    interactive()->error(muse::qtrc("project", "Cannot read file %1").arg(io::toNativeSeparators(filepath).toQString()).toStdString(),
                         muse::mtrc("project", "This file was saved using a newer version of Pianomania Composer. "
                                               "Please visit <a href=\"%1\">Pianomania</a> to obtain the latest version.")
                         .arg(u"https://pianomania.gg/docs/composer").toStdString());
}

bool ProjectActionsController::askIfUserAgreesToOpenCorruptedProject(const String& projectName, const std::string& errorText)
{
    std::string title = muse::mtrc("project", "File “%1” is corrupted").arg(projectName).toStdString();
    IInteractive::Text text;
    text.text = muse::trc("project", "This file contains errors that could cause Pianomania Composer to malfunction.");
    text.detailedText = errorText;

    IInteractive::ButtonData openAnywayBtn(IInteractive::Button::CustomButton, muse::trc("project", "Open anyway"), true /*accent*/);

    int btn = interactive()->warningSync(title, text, {
        interactive()->buttonData(IInteractive::Button::Cancel),
        openAnywayBtn
    }, openAnywayBtn.btn).button();

    return btn == openAnywayBtn.btn;
}

void ProjectActionsController::warnProjectCriticallyCorrupted(const String& projectName, const std::string& errorText)
{
    std::string title = muse::mtrc("project", "File “%1” is corrupted and cannot be opened").arg(projectName).toStdString();
    IInteractive::Text text;
    text.text = muse::trc("project", "Get help for this issue on Pianomania.");
    text.detailedText = errorText;

    IInteractive::ButtonData getHelpBtn(IInteractive::Button::CustomButton, muse::trc("project", "Get help"), true /*accent*/);

    interactive()->error(title, text, {
        interactive()->buttonData(IInteractive::Button::Cancel),
        getHelpBtn
    }, getHelpBtn.btn).onResolve(this, [this, getHelpBtn](const IInteractive::Result& res) {
        if (res.isButton(getHelpBtn.btn)) {
            interactive()->openUrl(configuration()->supportForumUrl());
        }
    });
}

void ProjectActionsController::warnProjectCannotBeOpened(const Ret& ret, const muse::io::path_t& filepath)
{
    std::string title = muse::mtrc("project", "Cannot read file %1").arg(io::toNativeSeparators(filepath).toString()).toStdString();
    std::string body;

    switch (ret.code()) {
    case int(engraving::Err::FileNotFound):
        body = muse::trc("project", "This file does not exist or cannot be accessed at the moment.");
        break;
    case int(engraving::Err::FileOpenError):
        body = muse::trc("project",
                         "This file could not be opened. Please make sure that Pianomania Composer has permission to read this file.");
        break;
    default:
        if (!ret.text().empty()) {
            body = ret.text();
        } else {
            body = muse::trc("project", "An error occurred while reading this file.");
        }
    }

    interactive()->error(title, body);
}

void ProjectActionsController::clearRecentScores()
{
    recentFilesController()->clearRecentFiles();
}

void ProjectActionsController::continueLastSession()
{
    const RecentFilesList& recentScorePaths = recentFilesController()->recentFilesList();

    if (recentScorePaths.empty()) {
        Ret ret = openPageIfNeed(HOME_PAGE_URI);
        if (!ret) {
            LOGE() << ret.toString();
        }
        return;
    }

    muse::io::path_t lastScorePath = recentScorePaths.front().path;
    openProject(lastScorePath);
}

void ProjectActionsController::exportScore()
{
    static const Uri EXPORT_URI("musescore://project/export");
    if (!interactive()->isOpened(EXPORT_URI).val) {
        interactive()->open(EXPORT_URI);
    }
}

bool ProjectActionsController::editComposerSongDetails()
{
    const auto project = currentNotationProject();
    if (!project) return false;
    auto meta = project->metaInfo();
    composer::SongDetails details;
    details.title = meta.title;
    for (const auto& role : { QString("composer"), QString("arranger") }) {
        const auto names = (role == "composer" ? meta.composer : meta.arranger).split('\n', Qt::SkipEmptyParts);
        for (const auto& name : names) details.credits.append({role, name.trimmed()});
    }
    if (meta.additionalTags.contains(composer::DifficultyRatingTag)) {
        bool valid = false;
        const int rating = meta.additionalTags.value(composer::DifficultyRatingTag).toString().toInt(&valid);
        if (valid && rating >= 0 && rating <= 999) details.difficultyRating = rating;
    }
    const auto selected = composer::chooseSongDetails(QApplication::activeWindow(), details);
    if (!selected) return false;
    meta.title = selected->title;
    QStringList composers, arrangers;
    for (const auto& credit : selected->credits) {
        (credit.role == "composer" ? composers : arrangers).append(credit.name);
    }
    meta.composer = composers.join('\n');
    meta.arranger = arrangers.join('\n');
    meta.additionalTags[composer::SongDetailsVersionTag] = "1";
    if (selected->difficultyRating) meta.additionalTags[composer::DifficultyRatingTag] = QString::number(*selected->difficultyRating);
    else meta.additionalTags.remove(composer::DifficultyRatingTag);
    project->setMetaInfo(meta, true);
    return true;
}

void ProjectActionsController::exportComposer()
{
    QWidget* parent = QApplication::activeWindow();
    try {
        auto notation = currentNotation();
        if (!notation || !notation->elements()->msScore()) return;
        if (!editComposerSongDetails()) return;
        auto sourceCompatibility = notationcompatibility::inventoryScore(notation->elements()->msScore(),
                                                                         currentNotationProject()->path().toQString());
        const auto sourceNavigation = compatibilityNavigation(globalContext(), notation);
        if (notationcompatibility::SummaryModel::instance()->setReport(sourceCompatibility.report, sourceNavigation.measureOrdinals, sourceNavigation.navigate)) {
            static const Uri compatibilityUri("musescore://project/notation-compatibility");
            if (!interactive()->isOpened(compatibilityUri).val) {
                UriQuery query(compatibilityUri);
                query.addParam("modal", Val(false));
                interactive()->open(query);
            }
        }
        QSettings settings;
        QString directory = settings.value("composer/exportDirectory", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
        if (!QDir(directory).exists()) directory = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        const auto exportPath = configuration()->defaultSavingFilePath(currentNotationProject(), {}, "pm");
        QString fileName = QFileInfo(exportPath.toQString()).fileName();
        fileName.replace(QRegularExpression("[<>:\"/\\\\|?*\\x00-\\x1f]"), "_");
        const auto selection = composer::chooseExport(parent, QDir(directory).filePath(fileName));
        if (!selection) return;
        const QString destination = selection->destination;

        // All engraving changes belong to this snapshot. The authored project stays unchanged.
        std::unique_ptr<mu::engraving::MasterScore> snapshot(notation->elements()->msScore()->masterScore()->clone());
        if (!snapshot) throw std::runtime_error("Could not prepare the score export.");
        pianomania::prepareComposerScore(snapshot.get());
        QTemporaryDir temporary;
        if (!temporary.isValid()) throw std::runtime_error("Could not create the export workspace.");
        QString base = temporary.path() + "/song";
        auto result = pianomania::exportPianomaniaBundle(snapshot.get(), muse::io::path_t("composer.mscz"),
            muse::io::path_t(base), muse::io::path_t(base + ".mei"), false);
        if (!result.ret) {
            if (result.ret.text().rfind("Score compatibility admission rejected:", 0) == 0) {
                LOGE() << result.ret.text();
                throw std::runtime_error("Pianomania couldn’t read this score. Check the score before trying again.");
            }
            if (result.ret.text().rfind("Coordinated Pianomania export", 0) == 0) {
                LOGE() << result.ret.text();
                throw std::runtime_error("Couldn’t export this score for Pianomania. Please try again.");
            }
            throw std::runtime_error("The score could not be exported for Pianomania.");
        }
        if (!pianomania::writePianomaniaManifest(muse::io::path_t(temporary.path() + "/manifest.json"), result.val))
            throw std::runtime_error("Could not create the export manifest.");
        auto compatibilityManifest = QJsonDocument::fromJson(QByteArray::fromStdString(result.val.manifestJson)).object();
        auto exportCompatibility = compatibilityManifest["notation_compatibility"].toObject();
        QFile exportedMei(base + ".mei");
        const QByteArray meiBytes = exportedMei.open(QIODevice::ReadOnly) ? exportedMei.readAll() : QByteArray();
        const auto exportNavigation = meiBytes.isEmpty() ? CompatibilityNavigation()
            : compatibilityNavigation(globalContext(), notation, meiBytes, snapshot.get());
        if (!exportCompatibility.isEmpty() && notationcompatibility::SummaryModel::instance()->setReport(exportCompatibility,
                exportNavigation.measureOrdinals, exportNavigation.navigate)) {
            static const Uri compatibilityUri("musescore://project/notation-compatibility");
            if (!interactive()->isOpened(compatibilityUri).val) {
                UriQuery query(compatibilityUri);
                query.addParam("modal", Val(false));
                interactive()->open(query);
            }
        }
        QVector<composer::Section> sections;
        auto add = [&](quint8 type, const QString& name) {
            QFile file(temporary.path() + "/" + name);
            if (!file.open(QIODevice::ReadOnly) || file.size() == 0 || file.size() > 128 * 1024 * 1024)
                throw std::runtime_error("A required export file is missing or too large.");
            sections.append({type, name, file.readAll()});
        };
        add(1, "song.mei"); add(2, "song.mid"); add(4, "manifest.json");
        if (result.val.repeatInfo.hasRepeats) add(3, "song-repeats.mid");
        sections.append({5, selection->background.fileName, selection->background.bytes});
        QString hash = composer::meiHash(sections);
        QByteArray license = composer::license(hash, parent);
        composer::validateLicense(license, composer::currentUid(), hash);
        sections.append({6, "license.json", license});
        QByteArray bytes = composer::encode(sections);
        QSaveFile output(destination);
        output.setDirectWriteFallback(false);
        if (!output.open(QIODevice::WriteOnly) || output.write(bytes) != bytes.size() || !output.commit())
            throw std::runtime_error("Could not save the Pianomania file. The previous file was preserved.");
        settings.setValue("composer/exportDirectory", QFileInfo(destination).absolutePath());
        QMessageBox saved(QMessageBox::Information, "Pianomania Composer",
                          "Your Pianomania file was exported successfully.", QMessageBox::Ok, parent);
        saved.setInformativeText("Saved to:\n" + QDir::toNativeSeparators(destination));
        saved.setTextFormat(Qt::PlainText);
        saved.exec();
    } catch (const std::exception& error) {
        QMessageBox::warning(parent, "Pianomania Composer", QString::fromUtf8(error.what()));
    }
}

void ProjectActionsController::exportPianomaniaAssets(const INotationPtr& notation, const muse::io::path_t& basePath,
                                                      bool exportPdf, bool exportMidi, bool exportMei)
{
    if (!notation) {
        return;
    }

    if (exportPdf) {
        exportProjectScenario()->exportScores({ notation }, basePath + ".pdf");
    }

    mu::engraving::Score* score = notation->elements()->msScore();
    if (!score) {
        return;
    }

    const pianomania::RepeatExportInfo repeatInfo = pianomania::analyzeRepeatExportInfo(score);
    const bool exportRpns = midiImportExportConfiguration()->isMidiExportRpns();

    std::unique_ptr<mu::engraving::MasterScore> noRepeatScore;
    if (repeatInfo.hasMultipleEndings && repeatInfo.finalEndingNumber > 0 && (exportMidi || exportMei)) {
        noRepeatScore = pianomania::buildNoRepeatScoreForFinalEnding(score, repeatInfo.finalEndingNumber);
        if (!noRepeatScore) {
            LOGE() << "Failed to build no-repeat score for Pianomania export: " << notation->name();
        }
    }

    if (exportMidi) {
        mu::engraving::Score* baseScore
            = noRepeatScore ? static_cast<mu::engraving::Score*>(noRepeatScore.get()) : score;
        pianomania::writeMidiFile(baseScore, basePath + ".mid", false, exportRpns);

        if (repeatInfo.hasRepeats) {
            pianomania::writeMidiFile(score, basePath + "-repeats.mid", true, exportRpns);
        }
    }

    if (exportMei) {
        exportProjectScenario()->exportScores({ notation }, basePath + ".mei");
    }
}

void ProjectActionsController::exportPianomania()
{
    // One command covers both scopes: the open score, or every score in a folder
    // tree. The same output choices apply either way, so a batch re-export
    // produces the same file set as exporting each score on its own.
    INotationPtr currentScore = currentNotation();

    QDialog dlg;
    dlg.setWindowTitle(muse::qtrc("project/export", "Pianomania Export"));

    QVBoxLayout layout(&dlg);

    QRadioButton currentScoreRadio(muse::qtrc("project/export", "Current score"));
    currentScoreRadio.setEnabled(currentScore != nullptr);
    layout.addWidget(&currentScoreRadio);

    QRadioButton folderRadio(muse::qtrc("project/export", "Every score in a folder"));
    layout.addWidget(&folderRadio);

    // Without an open score the folder scope is the only thing that can run.
    currentScoreRadio.setChecked(currentScore != nullptr);
    folderRadio.setChecked(currentScore == nullptr);

    QCheckBox pdfCheck(muse::qtrc("project/export", "PDF"));
    pdfCheck.setChecked(true);
    layout.addWidget(&pdfCheck);

    QCheckBox midiCheck(muse::qtrc("project/export", "MIDI"));
    midiCheck.setChecked(true);
    layout.addWidget(&midiCheck);

    QCheckBox meiCheck(muse::qtrc("project/export", "MEI"));
    meiCheck.setChecked(true);
    layout.addWidget(&meiCheck);

    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout.addWidget(&buttons);

    QObject::connect(&buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(&buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    const bool exportPdf = pdfCheck.isChecked();
    const bool exportMidi = midiCheck.isChecked();
    const bool exportMei = meiCheck.isChecked();

    if (!exportPdf && !exportMidi && !exportMei) {
        return;
    }

    if (currentScoreRadio.isChecked()) {
        if (!currentScore) {
            return;
        }

        muse::io::path_t dir = interactive()->selectDirectory(muse::trc("project/export", "Select export folder"), "");
        if (dir.empty()) {
            return;
        }

        muse::io::path_t basePath = dir + "/" + muse::io::escapeFileName(currentScore->name());
        exportPianomaniaAssets(currentScore, basePath, exportPdf, exportMidi, exportMei);
        return;
    }

    muse::io::path_t root = interactive()->selectDirectory(muse::trc("project/export", "Select root folder"), "");
    if (root.empty()) {
        return;
    }

    RetVal<muse::io::paths_t> files = fileSystem()->scanFiles(root, { "*.mscz" }, muse::io::ScanMode::FilesInCurrentDirAndSubdirs);
    if (!files.ret) {
        return;
    }

    // Each score's output lands beside its own file so a tree keeps its shape.
    for (const muse::io::path_t& file : files.val) {
        RetVal<INotationProjectPtr> projRv = loadProject(file);
        if (!projRv.ret) {
            continue;
        }

        INotationPtr notation = projRv.val->masterNotation()->notation();
        muse::io::path_t basePath = muse::io::dirpath(file) + "/" + muse::io::escapeFileName(notation->name());

        exportPianomaniaAssets(notation, basePath, exportPdf, exportMidi, exportMei);
    }
}

void ProjectActionsController::printScore()
{
    INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return;
    }

    printProvider()->printNotation(notation);
}

async::Promise<io::path_t> ProjectActionsController::selectScoreOpeningFile() const
{
    std::string allExt = "*.mscz *.mxl *.musicxml *.xml *.mid *.midi *.kar *.md *.mgu *.sgu *.cap *.capx "
                         "*.ove *.scw *.bmw *.bww *.gtp *.gp3 *.gp4 *.gp5 *.gpx *.gp *.ptb *.mei *.mnx *.json *.tef *.mscx *.mscs *.mscz~";

    std::vector<std::string> filter { muse::trc("project", "All supported files") + " (" + allExt + ")",
                                      muse::trc("project", "Composer scores") + " (*.mscz)",
                                      muse::trc("project", "MusicXML files") + " (*.mxl *.musicxml *.xml)",
                                      muse::trc("project", "MIDI files") + " (*.mid *.midi *.kar)",
                                      muse::trc("project", "MNX files (experimental)") + " (*.mnx *.json)",
                                      muse::trc("project", "MuseData files") + " (*.md)",
                                      muse::trc("project", "Capella files") + " (*.cap *.capx)",
                                      muse::trc("project", "BB files (experimental)") + " (*.mgu *.sgu)",
                                      muse::trc("project", "Overture / Score Writer files (experimental)") + " (*.ove *.scw)",
                                      muse::trc("project", "Bagpipe Music Writer files (experimental)") + " (*.bmw *.bww)",
                                      muse::trc("project", "Guitar Pro files") + " (*.gtp *.gp3 *.gp4 *.gp5 *.gpx *.gp)",
                                      muse::trc("project", "Power Tab Editor files (experimental)") + " (*.ptb)",
                                      muse::trc("project", "MEI files") + " (*.mei)",
                                      muse::trc("project", "TablEdit files (experimental)") + " (*.tef)",
                                      muse::trc("project", "Uncompressed Composer folders (experimental)") + " (*.mscx)",
                                      muse::trc("project", "Composer developer files") + " (*.mscs)",
                                      muse::trc("project", "Composer backup files") + " (*.mscz~)" };

    muse::io::path_t defaultDir = configuration()->lastOpenedProjectsPath();

    if (defaultDir.empty()) {
        defaultDir = configuration()->userProjectsPath();
    }

    if (defaultDir.empty()) {
        defaultDir = configuration()->defaultUserProjectsPath();
    }

    return interactive()->selectOpeningFile(muse::trc("project", "Open"), defaultDir, filter);
}

bool ProjectActionsController::hasSelection() const
{
    return currentNotationSelection() ? !currentNotationSelection()->isNone() : false;
}

void ProjectActionsController::openProjectProperties()
{
    interactive()->open(PROJECT_PROPERTIES_URI);
}
