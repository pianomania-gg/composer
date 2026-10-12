// SPDX-License-Identifier: GPL-3.0-only
#include "../../internal/composersongdetails.h"
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QTest>
#include <QTimer>

using namespace mu::project::composer;
class SongDetailsTest : public QObject {
    Q_OBJECT
private slots:
    void creditsAndDifficultyApply() {
        QTimer::singleShot(0, [] {
            auto* dialog = QApplication::activeModalWidget();
            dialog->findChild<QLineEdit*>("songDetailsTitle")->setText("Study & variations");
            dialog->findChild<QPushButton*>("addSongCredit")->click();
            dialog->findChildren<QComboBox*>("songCreditRole").last()->setCurrentIndex(1);
            dialog->findChildren<QLineEdit*>("songCreditName").last()->setText("New arranger");
            dialog->findChild<QCheckBox*>("songDetailsDrEnabled")->setChecked(true);
            dialog->findChild<QSlider*>("songDetailsDrSlider")->setValue(235);
            QCOMPARE(dialog->findChild<QSpinBox*>("songDetailsDr")->value(), 235);
            QCOMPARE(dialog->findChild<QLabel*>("songCardArrangers")->text(), QString("Arranged by New arranger"));
            dialog->findChild<QPushButton*>("applySongDetails")->click();
        });
        const auto result = chooseSongDetails(nullptr, {"Study", {{"composer", "Original composer"}}, {}});
        QVERIFY(result.has_value());
        QCOMPARE(result->title, QString("Study & variations"));
        QCOMPARE(result->credits.size(), 2);
        QCOMPARE(result->credits[1].role, QString("arranger"));
        QCOMPARE(result->credits[1].name, QString("New arranger"));
        QVERIFY(result->difficultyRating.has_value());
        QCOMPARE(*result->difficultyRating, 235);
    }
    void unsetDifficultyIsDistinctFromZero() {
        QTimer::singleShot(0, [] { auto* dialog = QApplication::activeModalWidget();
            dialog->findChild<QPushButton*>("applySongDetails")->click(); });
        const auto zero = chooseSongDetails(nullptr, {"Study", {}, 0});
        QVERIFY(zero->difficultyRating.has_value()); QCOMPARE(*zero->difficultyRating, 0);
        QTimer::singleShot(0, [] { auto* dialog = QApplication::activeModalWidget();
            dialog->findChild<QCheckBox*>("songDetailsDrEnabled")->setChecked(false);
            dialog->findChild<QPushButton*>("applySongDetails")->click(); });
        const auto unset = chooseSongDetails(nullptr, {"Study", {}, 235});
        QVERIFY(!unset->difficultyRating.has_value());
    }
    void validationAndCancelPreserveInput() {
        const SongDetails original {"Study", {{"composer", "Composer"}}, 165};
        QTimer::singleShot(0, [] { auto* dialog = QApplication::activeModalWidget();
            dialog->findChild<QLineEdit*>("songDetailsTitle")->clear();
            dialog->findChild<QPushButton*>("applySongDetails")->click();
            QVERIFY(dialog->findChild<QLabel*>("songDetailsError")->isVisible());
            dialog->findChild<QLineEdit*>("songDetailsTitle")->setText("Changed");
            dialog->findChild<QPushButton*>("addSongCredit")->click();
            dialog->findChild<QPushButton*>("applySongDetails")->click();
            QCOMPARE(dialog->findChild<QLabel*>("songDetailsError")->text(), QString("Enter a name or remove the empty row."));
            qobject_cast<QDialog*>(dialog)->reject(); });
        QVERIFY(!chooseSongDetails(nullptr, original).has_value());
        QCOMPARE(original.title, QString("Study")); QCOMPARE(*original.difficultyRating, 165);
    }
};
QTEST_MAIN(SongDetailsTest)
#include "details_test.moc"
