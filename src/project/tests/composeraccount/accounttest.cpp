// SPDX-License-Identifier: GPL-3.0-only
#include <QApplication>
#include <QTest>
#include <QFontDatabase>
// Exercise the actual session and dialog code with an in-memory account; never
// read or alter the user's credential vault, and intercept all browser launches.
#include "../../internal/composersession.cpp"

using namespace mu::project::composer;

class AccountTest : public QObject {
    Q_OBJECT
    QUrl openedUrl;
private slots:
    void initTestCase() {
#ifdef Q_OS_WIN
        QFontDatabase::addApplicationFont("C:/Windows/Fonts/segoeui.ttf");
        QApplication::setFont(QFont("Segoe UI", 10));
#endif
        if (qEnvironmentVariableIsSet("COMPOSER_TEST_DARK")) {
            auto palette = QApplication::palette();
            palette.setColor(QPalette::Window, QColor("#2b2b2b"));
            palette.setColor(QPalette::WindowText, QColor("#eeeeee"));
            palette.setColor(QPalette::Button, QColor("#484848"));
            palette.setColor(QPalette::ButtonText, QColor("#eeeeee"));
            QApplication::setPalette(palette);
        }
    }
    void init() {
        loaded = true;
        session = {};
        selectAccountOnNextSignIn = false;
        openedUrl = QUrl();
        QDesktopServices::setUrlHandler("https", this, "openUrl");
    }
    void cleanup() { QDesktopServices::unsetUrlHandler("https"); }
    void statusDoesNotCreateAccount() {
        AccountState state;
        state.refresh();
        QVERIFY(!state.signedIn());
        QVERIFY(state.email().isEmpty());
        QVERIFY(state.error().isEmpty());
        QVERIFY(currentUid().isEmpty());
        QVERIFY(session.isEmpty());
    }
    void signInAfterStatusCheckOpensBrowser() {
        accountState()->refresh();
        // End the browser wait without a network request; dismiss its resulting
        // cancellation notice. A mistaken account dialog is also dismissed.
        QTimer dismiss;
        QObject::connect(&dismiss, &QTimer::timeout, &dismiss, [&dismiss] {
            for (auto widget : QApplication::topLevelWidgets()) {
                if (auto progress = qobject_cast<QProgressDialog*>(widget)) {
                    if (auto cancel = progress->findChild<QPushButton*>()) cancel->click();
                    return;
                }
                if (auto message = qobject_cast<QMessageBox*>(widget)) {
                    dismiss.stop();
                    message->reject();
                    return;
                }
            }
        });
        dismiss.start(50);
        showAccount(nullptr);
        QCOMPARE(openedUrl.path(), QString("/composer/authorize"));
        QCOMPARE(QUrlQuery(openedUrl).queryItemValue("environment"), QString("account-regression"));
        QVERIFY(session.isEmpty());
        QVERIFY(!accountState()->signedIn());
    }
    void savedAccountSurvivesStatusCheck() {
        session = {{"uid", "test-user"}, {"email", "test@example.invalid"},
                   {"refreshToken", "test-token"}, {"apiKey", "test-key"}};
        const auto original = session;
        AccountState state;
        QVERIFY(state.signedIn());
        QCOMPARE(state.email(), QString("test@example.invalid"));
        QCOMPARE(currentUid(), QString("test-user"));
        QCOMPARE(session, original);
    }
    void accountDialogShowsIdentityAndManagesAccount() {
        session = {{"uid", "test-user"}, {"email", "pianist@example.com"},
                   {"refreshToken", "test-token"}, {"apiKey", "test-key"}};
        const auto original = session;
        bool inspected = false;
        QTimer::singleShot(50, [&] {
            for (auto widget : QApplication::topLevelWidgets()) {
                auto dialog = qobject_cast<QDialog*>(widget);
                if (!dialog || dialog->objectName() != "composerAccountDialog") continue;
                inspected = true;
                auto email = dialog->findChild<QLabel*>("accountEmail");
                QVERIFY(email);
                QCOMPARE(email->text(), QString("pianist@example.com"));
                QVERIFY(email->isVisible());
                QVERIFY(dialog->grab().save("composer-account-dialog.png"));
                QPushButton* done = nullptr;
                for (auto button : dialog->findChildren<QPushButton*>()) {
                    if (button->text() == "Manage account") button->click();
                    if (button->text() == "Done") done = button;
                }
                QVERIFY(done);
                QVERIFY(done->isDefault());
                done->click();
            }
        });
        showAccount(nullptr);
        QVERIFY(inspected);
        QCOMPARE(openedUrl.path(), QString("/beta-portal/account"));
        QCOMPARE(session, original);
    }
public slots:
    void openUrl(const QUrl& url) { openedUrl = url; }
};
QTEST_MAIN(AccountTest)
#include "accounttest.moc"
