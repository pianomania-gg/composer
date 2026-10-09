// SPDX-License-Identifier: GPL-3.0-only
#include "composerbackgrounddialog.h"
#include <QBuffer>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QImageReader>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QStackedWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <stdexcept>

static void initializeBackgroundResources()
{
    static const bool initialized = [] {
        Q_INIT_RESOURCE(composer_backgrounds);
        return true;
    }();
    Q_UNUSED(initialized);
}

namespace mu::project::composer {
BackgroundSelection readBackground(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly) || file.size() <= 0 || file.size() > 20 * 1024 * 1024)
        throw std::runtime_error("Choose a PNG or JPEG image under 20 MB.");
    QByteArray bytes = file.readAll();
    if (file.error() != QFileDevice::NoError)
        throw std::runtime_error("The background image could not be read. Choose the image again.");
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::ReadOnly);
    QImageReader reader(&buffer);
    const auto format = reader.format().toLower();
    const auto size = reader.size();
    if ((format != "png" && format != "jpeg" && format != "jpg") || !size.isValid()
        || size.width() > 8192 || size.height() > 8192 || reader.read().isNull())
        throw std::runtime_error("Choose a PNG or JPEG image no larger than 8192 pixels per side.");
    return {bytes, format == "png" ? "cover.png" : "cover.jpg"};
}

static QPixmap croppedPreview(const QByteArray& bytes, const QSize& size)
{
    QPixmap pixmap;
    if (!pixmap.loadFromData(bytes)) throw std::runtime_error("The image preview could not be loaded.");
    const auto scaled = pixmap.scaled(size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    return scaled.copy((scaled.width() - size.width()) / 2, (scaled.height() - size.height()) / 2,
                       size.width(), size.height());
}

std::optional<ExportSelection> chooseExport(QWidget* parent, const QString& suggestedPath)
{
    initializeBackgroundResources();
    QDialog dialog(parent);
    dialog.setObjectName("composerExportDialog");
    dialog.setWindowTitle("Export Pianomania file");
    dialog.setMinimumWidth(744);
    dialog.setStyleSheet(QString(
        "QLabel { font-size: 14px; }"
        "QLabel#backgroundHeading { font-size: 23px; font-weight: 600; }"
        "QLabel#exportSteps { font-size: 12px; }"
        "QPushButton { font-size: 14px; padding: 8px 16px; border: 1px solid palette(mid); border-radius: 5px; background-color: palette(button); }"
        "QPushButton:focus { border: 1px solid palette(highlight); }"
        "QPushButton#continueExport { background-color: %1; color: %2; border: 1px solid %1; }"
        "QPushButton#continueExport:disabled { background: palette(mid); }"
        "QToolButton { padding: 6px; border: 2px solid transparent; border-radius: 5px; }"
        "QToolButton:hover { background: palette(alternate-base); border-color: palette(mid); }"
        "QToolButton:checked, QToolButton:focus { border-color: palette(highlight); }"
        "QLineEdit { padding: 12px; border-radius: 5px; background: palette(base); }")
        .arg(dialog.palette().color(QPalette::Highlight).name(), dialog.palette().color(QPalette::HighlightedText).name()));
    auto* layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(28, 24, 28, 24);
    layout->setSpacing(16);
    auto* steps = new QLabel("1 / 2", &dialog);
    steps->setObjectName("exportSteps");
    layout->addWidget(steps);
    auto* title = new QLabel("Choose a song background", &dialog);
    title->setObjectName("backgroundHeading");
    layout->addWidget(title);

    auto* pages = new QStackedWidget(&dialog);
    pages->setObjectName("exportPages");
    layout->addWidget(pages);
    auto* artworkPage = new QWidget(pages);
    auto* artwork = new QVBoxLayout(artworkPage);
    artwork->setContentsMargins(0, 0, 0, 0);
    artwork->setSpacing(12);
    auto* explanation = new QLabel("This image appears on your song card and behind the music in Pianomania.", artworkPage);
    explanation->setWordWrap(true);
    artwork->addWidget(explanation);
    auto* preview = new QLabel(artworkPage);
    preview->setObjectName("backgroundPreview");
    preview->setFixedSize(672, 252);
    preview->setAlignment(Qt::AlignCenter);
    artwork->addWidget(preview, 0, Qt::AlignHCenter);
    auto* credit = new QLabel(artworkPage);
    credit->setObjectName("backgroundCredit");
    credit->setTextFormat(Qt::PlainText);
    credit->setWordWrap(true);
    artwork->addWidget(credit);
    auto* choices = new QHBoxLayout;
    choices->setSpacing(8);
    artwork->addLayout(choices);
    auto* custom = new QPushButton("Choose your own…", artworkPage);
    custom->setObjectName("customBackground");
    artwork->addWidget(custom, 0, Qt::AlignLeft);
    pages->addWidget(artworkPage);

    auto* savePage = new QWidget(pages);
    auto* save = new QVBoxLayout(savePage);
    save->setContentsMargins(0, 0, 0, 0);
    save->setSpacing(16);
    auto* savePreview = new QLabel(savePage);
    savePreview->setFixedSize(672, 252);
    save->addWidget(savePreview, 0, Qt::AlignHCenter);
    auto* destinationLabel = new QLabel("Saved to:", savePage);
    save->addWidget(destinationLabel);
    auto* destination = new QLineEdit(QDir::toNativeSeparators(suggestedPath), savePage);
    destination->setObjectName("exportDestination");
    destination->setReadOnly(true);
    destination->setAccessibleName(destinationLabel->text());
    destinationLabel->setBuddy(destination);
    save->addWidget(destination);
    auto* browse = new QPushButton("Choose save location…", savePage);
    browse->setObjectName("chooseExportDestination");
    save->addWidget(browse, 0, Qt::AlignLeft);
    save->addStretch();
    pages->addWidget(savePage);

    auto* error = new QLabel(&dialog);
    error->setObjectName("backgroundError");
    error->setWordWrap(true);
    error->setTextFormat(Qt::PlainText);
    error->setMinimumHeight(22);
    layout->addWidget(error);
    auto* buttons = new QHBoxLayout;
    auto* cancel = new QPushButton(QDialog::tr("Cancel"), &dialog);
    buttons->addWidget(cancel);
    buttons->addStretch();
    auto* back = new QPushButton(QDialog::tr("Back"), &dialog);
    buttons->addWidget(back);
    back->setObjectName("backExport");
    back->hide();
    auto* next = new QPushButton(QDialog::tr("Next"), &dialog);
    buttons->addWidget(next);
    next->setObjectName("continueExport");
    next->setDefault(true);
    next->setEnabled(false);
    layout->addLayout(buttons);

    std::optional<BackgroundSelection> selected;
    QVector<QToolButton*> stockButtons;
    auto select = [&](const QString& path, const QString& caption, QToolButton* choice) {
        try {
            auto candidate = readBackground(path);
            const auto pixmap = croppedPreview(candidate.bytes, preview->size());
            preview->setPixmap(pixmap);
            savePreview->setPixmap(pixmap);
            selected = std::move(candidate);
            credit->setText(caption);
            for (auto* button : stockButtons) button->setChecked(button == choice);
            error->clear();
            next->setEnabled(true);
        } catch (const std::exception& exception) {
            error->setText(QString::fromUtf8(exception.what()));
        }
    };
    struct Stock { const char* id; const char* title; const char* artist; };
    const Stock stocks[] = {
        {"courtyard", "Moscow Courtyard", "Vasily Polenov"},
        {"windmill", "The Mill of Montmartre", "Georges Michel"},
        {"library", "Old Library", "Stanislaw Zhukovsky"},
        {"river", "River", "Ladislav Mednyanszky"},
    };
    for (const auto& stock : stocks) {
        const QString path = QString(":/composer/backgrounds/%1.jpg").arg(stock.id);
        auto* button = new QToolButton(artworkPage);
        button->setObjectName(QString(stock.id) + "Background");
        button->setText(QString::fromUtf8(stock.title));
        button->setAccessibleName(button->text());
        button->setToolTip(QString::fromUtf8(stock.artist));
        button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        button->setIconSize(QSize(146, 82));
        button->setCheckable(true);
        stockButtons.append(button);
        choices->addWidget(button);
        try {
            button->setIcon(croppedPreview(readBackground(path).bytes, button->iconSize()));
        } catch (const std::exception& exception) {
            button->setEnabled(false);
            error->setText(QString::fromUtf8(exception.what()));
        }
        const QString caption = button->text() + " · " + QString::fromUtf8(stock.artist);
        QObject::connect(button, &QToolButton::clicked, &dialog, [&, path, caption, button] {
            select(path, caption, button);
        });
    }
    QObject::connect(custom, &QPushButton::clicked, &dialog, [&] {
        const auto path = QFileDialog::getOpenFileName(&dialog, "Choose a song background", {}, "Images (*.png *.jpg *.jpeg)");
        if (!path.isEmpty()) select(path, QFileInfo(path).fileName(), nullptr);
    });
    QObject::connect(browse, &QPushButton::clicked, &dialog, [&] {
        QFileDialog picker(&dialog, "Choose save location…", destination->text(), "Pianomania (*.pm)");
        picker.setAcceptMode(QFileDialog::AcceptSave);
        picker.setDefaultSuffix("pm");
        // Confirm at Export, including when the prefilled destination already exists.
        picker.setOption(QFileDialog::DontConfirmOverwrite);
        if (picker.exec() != QDialog::Accepted) return;
        QString path = picker.selectedFiles().first();
        if (!path.endsWith(".pm", Qt::CaseInsensitive)) path += ".pm";
        destination->setText(QDir::toNativeSeparators(path));
    });
    QObject::connect(back, &QPushButton::clicked, &dialog, [&] {
        pages->setCurrentIndex(0);
        title->setText("Choose a song background");
        steps->setText("1 / 2");
        next->setText(QDialog::tr("Next"));
        back->hide();
        next->setFocus();
    });
    QObject::connect(next, &QPushButton::clicked, &dialog, [&] {
        if (!selected) return;
        if (pages->currentIndex() == 0) {
            pages->setCurrentIndex(1);
            title->setText("Choose save location…");
            steps->setText("2 / 2");
            next->setText("Export");
            back->show();
            error->clear();
            browse->setFocus();
            return;
        }
        const auto path = QDir::fromNativeSeparators(destination->text());
        if (path.isEmpty()) return;
        // Let the native save dialog own overwrite wording and confirmation.
        if (QFileInfo::exists(path)) {
            QFileDialog confirm(&dialog, "Choose save location…", path, "Pianomania (*.pm)");
            confirm.setAcceptMode(QFileDialog::AcceptSave);
            confirm.setDefaultSuffix("pm");
            if (confirm.exec() != QDialog::Accepted) return;
            QString confirmed = confirm.selectedFiles().first();
            // Reopen with the normalized extension so overwrite checks target the actual file.
            if (!confirmed.endsWith(".pm", Qt::CaseInsensitive)) {
                destination->setText(QDir::toNativeSeparators(confirmed + ".pm"));
                return;
            }
            destination->setText(QDir::toNativeSeparators(confirmed));
        }
        dialog.accept();
    });
    QObject::connect(cancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    stockButtons.first()->click();
    if (dialog.exec() != QDialog::Accepted || !selected) return std::nullopt;
    return ExportSelection{*selected, QDir::fromNativeSeparators(destination->text())};
}
}
