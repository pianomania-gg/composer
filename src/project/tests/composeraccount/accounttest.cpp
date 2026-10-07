// SPDX-License-Identifier: GPL-3.0-only
#include <QApplication>
#include <QTest>
// Exercise the actual session and dialog code with an in-memory account; never
// read or alter the user's credential vault, and intercept all browser launches.
#include "../../internal/composersession.cpp"

using namespace mu::project::composer;

class AccountTest : public QObject {
    Q_OBJECT
    QUrl openedUrl;
private slots:
    void init() {
        loaded = true;
        session = {};
        selectAccountOnNextSignIn = false;
        openedUrl = {};
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
        QTimer::singleShot(50, [] {
            for (auto widget : QApplication::topLevelWidgets()) {
                if (auto progress = qobject_cast<QProgressDialog*>(widget)) {
                    QMetaObject::invokeMethod(progress, "canceled");
                }
                if (auto message = qobject_cast<QMessageBox*>(widget)) message->reject();
            }
            QTimer::singleShot(50, [] {
                for (auto widget : QApplication::topLevelWidgets())
                    if (auto message = qobject_cast<QMessageBox*>(widget)) message->reject();
            });
        });
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
public slots:
    void openUrl(const QUrl& url) { openedUrl = url; }
};
QTEST_MAIN(AccountTest)
#include "accounttest.moc"
