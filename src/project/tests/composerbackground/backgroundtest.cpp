// SPDX-License-Identifier: GPL-3.0-only
#include <QtTest>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QStackedWidget>
#include <QFileDialog>
#include <QFontDatabase>
#include <QPushButton>
#include <QTemporaryDir>
#include <QTimer>
#include "../../internal/composerbackgrounddialog.h"
using namespace mu::project::composer;

class BackgroundTest : public QObject {
    Q_OBJECT
private slots:
    // Test value: Detects missing bundled art and incorrect selection at the exported cover bytes boundary.
    void builtInSelection()
    {
        QTemporaryDir temp;
        QTimer::singleShot(0, [] {
            auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
            QVERIFY(dialog);
            auto* preview = dialog->findChild<QLabel*>("backgroundPreview");
            QVERIFY(preview && !preview->pixmap().isNull());
            auto* next = dialog->findChild<QPushButton*>("continueExport");
            QVERIFY(next && next->isEnabled());
            QVERIFY(dialog->grab().save("composer-background-courtyard.png"));
            dialog->findChild<QToolButton*>("windmillBackground")->click();
            QVERIFY(dialog->findChild<QLabel*>("backgroundCredit")->text().contains("Georges Michel"));
            QVERIFY(dialog->grab().save("composer-background-windmill.png"));
            next->click();
            QCOMPARE(dialog->findChild<QStackedWidget*>("exportPages")->currentIndex(), 1);
            QVERIFY(dialog->grab().save("composer-export-location.png"));
            dialog->findChild<QPushButton*>("backExport")->click();
            QVERIFY(dialog->findChild<QToolButton*>("windmillBackground")->isChecked());
            next->click();
            next->click();
        });
        auto result = chooseExport(nullptr, temp.filePath("song.pm"));
        QVERIFY(result);
        QCOMPARE(result->background.fileName, QString("cover.jpg"));
        QCOMPARE(result->background.bytes, readBackground(":/composer/backgrounds/windmill.jpg").bytes);
        QCOMPARE(result->destination, temp.filePath("song.pm"));
        for (const auto& id : {"courtyard", "windmill", "library", "river"}) {
            QVERIFY(!readBackground(QString(":/composer/backgrounds/%1.jpg").arg(id)).bytes.isEmpty());
        }
    }

    // Test value: Detects cancelled image selection leaking an export at the dialog result boundary.
    void cancelledSelection()
    {
        QTimer::singleShot(0, [] {
            qobject_cast<QDialog*>(QApplication::activeModalWidget())->reject();
        });
        QVERIFY(!chooseExport(nullptr, "cancelled.pm"));
    }

    // Test value: Catches cancelled save steps accidentally proceeding to package generation.
    void cancelledSaveStep()
    {
        QTimer::singleShot(0, [] {
            auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
            dialog->findChild<QPushButton*>("continueExport")->click();
            dialog->reject();
        });
        QVERIFY(!chooseExport(nullptr, "cancelled.pm"));
    }

    // Test value: Protects custom artwork bytes and selection across native-picker cancellation and invalid input.
    void customArtwork()
    {
        QTemporaryDir temp;
        const auto path = temp.filePath("custom.png");
        QImage image(128, 72, QImage::Format_RGB32);
        image.fill(Qt::blue);
        QVERIFY(image.save(path));
        const auto corruptPath = temp.filePath("corrupt.png");
        QFile corrupt(corruptPath);
        QVERIFY(corrupt.open(QIODevice::WriteOnly));
        corrupt.write("invalid");
        corrupt.close();
        QTimer::singleShot(0, [&] {
            auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
            auto* custom = dialog->findChild<QPushButton*>("customBackground");
            auto pick = [&](const QString& file) {
                QTimer::singleShot(0, [file] {
                    auto* picker = qobject_cast<QFileDialog*>(QApplication::activeModalWidget());
                    QVERIFY(picker);
                    picker->selectFile(file);
                    QMetaObject::invokeMethod(picker, "accept", Qt::DirectConnection);
                });
                custom->click();
            };
            pick(path);
            QCOMPARE(dialog->findChild<QLabel*>("backgroundCredit")->text(), QString("custom.png"));
            QVERIFY(!dialog->findChild<QToolButton*>("courtyardBackground")->isChecked());
            pick(corruptPath);
            QVERIFY(!dialog->findChild<QLabel*>("backgroundError")->text().isEmpty());
            QCOMPARE(dialog->findChild<QLabel*>("backgroundCredit")->text(), QString("custom.png"));
            QTimer::singleShot(0, [] { qobject_cast<QDialog*>(QApplication::activeModalWidget())->reject(); });
            custom->click();
            auto* next = dialog->findChild<QPushButton*>("continueExport");
            next->click();
            next->click();
        });
        auto result = chooseExport(nullptr, temp.filePath("custom.pm"));
        QVERIFY(result);
        QCOMPARE(result->background.bytes, readBackground(path).bytes);
        QCOMPARE(result->background.fileName, QString("cover.png"));
    }

    // Test value: Protects the actual export path when users omit the extension or cancel destination changes.
    void saveDestination()
    {
        QTemporaryDir temp;
        QTimer::singleShot(0, [&] {
            auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
            auto* next = dialog->findChild<QPushButton*>("continueExport");
            next->click();
            auto* browse = dialog->findChild<QPushButton*>("chooseExportDestination");
            QTimer::singleShot(0, [&] {
                auto* picker = qobject_cast<QFileDialog*>(QApplication::activeModalWidget());
                QVERIFY(picker);
                picker->selectFile(temp.filePath("renamed"));
                QMetaObject::invokeMethod(picker, "accept", Qt::DirectConnection);
            });
            browse->click();
            QTimer::singleShot(0, [] { qobject_cast<QDialog*>(QApplication::activeModalWidget())->reject(); });
            browse->click();
            next->click();
        });
        auto result = chooseExport(nullptr, temp.filePath("song.pm"));
        QVERIFY(result);
        QCOMPARE(result->destination, temp.filePath("renamed.pm"));
        QVERIFY(!QFileInfo::exists(result->destination));
    }

    // Test value: Protects existing packages when the final overwrite flow is cancelled.
    void cancelledOverwrite()
    {
        QTemporaryDir temp;
        const auto path = temp.filePath("existing.pm");
        QFile previous(path);
        QVERIFY(previous.open(QIODevice::WriteOnly));
        previous.write("previous package");
        previous.close();
        QTimer::singleShot(0, [&] {
            auto* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
            auto* next = dialog->findChild<QPushButton*>("continueExport");
            next->click();
            QTimer::singleShot(0, [] {
                auto* picker = qobject_cast<QFileDialog*>(QApplication::activeModalWidget());
                QVERIFY(picker);
                picker->reject();
            });
            next->click();
            QVERIFY(dialog->isVisible());
            dialog->reject();
        });
        QVERIFY(!chooseExport(nullptr, path));
        QVERIFY(previous.open(QIODevice::ReadOnly));
        QCOMPARE(previous.readAll(), QByteArray("previous package"));
    }

    // Test value: Detects extension spoofing and corrupt images at the package cover input boundary.
    void validatesImageContent()
    {
        QTemporaryDir temp;
        const auto path = temp.filePath("image.jpg");
        QImage image(32, 18, QImage::Format_RGB32);
        image.fill(Qt::blue);
        QVERIFY(image.save(path, "PNG"));
        QCOMPARE(readBackground(path).fileName, QString("cover.png"));
        QFile corrupt(path);
        QVERIFY(corrupt.open(QIODevice::WriteOnly | QIODevice::Truncate));
        corrupt.write("not an image");
        corrupt.close();
        QVERIFY_EXCEPTION_THROWN(readBackground(path), std::runtime_error);
    }

    // Test value: Detects oversized artwork reaching decode at the bounded image input boundary.
    void rejectsOversizedDimensions()
    {
        QTemporaryDir temp;
        const auto path = temp.filePath("oversized.png");
        QImage image(8193, 1, QImage::Format_RGB32);
        image.fill(Qt::blue);
        QVERIFY(image.save(path));
        QVERIFY_EXCEPTION_THROWN(readBackground(path), std::runtime_error);
    }
};
int main(int argc, char** argv)
{
    QApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
    QApplication application(argc, argv);
#ifdef Q_OS_WIN
    // The offscreen Qt adapter has no system font database on Windows.
    QFontDatabase::addApplicationFont(QDir(qEnvironmentVariable("WINDIR")).filePath("Fonts/segoeui.ttf"));
    application.setFont(QFont("Segoe UI", 10));
#endif
    BackgroundTest test;
    return QTest::qExec(&test, argc, argv);
}
#include "backgroundtest.moc"
