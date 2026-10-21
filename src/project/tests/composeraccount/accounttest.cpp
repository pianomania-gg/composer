// SPDX-License-Identifier: GPL-3.0-only
#include <QApplication>
#include <QTest>
#include <QFontDatabase>
#include <QUuid>
#ifdef Q_OS_MACOS
#include <dlfcn.h>
#endif
// Exercise the actual session and dialog code with an in-memory account and
// intercept all browser launches. The macOS vault regression uses only a
// randomized dummy service with exact cleanup, never the product vault name.
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
#elif defined(Q_OS_MACOS)
        QApplication::setFont(QFont("Helvetica", 10));
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
#ifdef Q_OS_MACOS
    void modernKeychainApiReadsUpdatesAndDeletesLegacyEntry() {
        const QByteArray name = "PianomaniaComposer/account-regression/"
                                + QUuid::createUuid().toByteArray(QUuid::WithoutBraces);
        struct ExactItemCleanup {
            QByteArray name;
            ~ExactItemCleanup() noexcept {
                try {
                    macVaultWrite(name, {});
                } catch (const std::exception& failure) {
                    const QByteArray message = "Failed to clean up Keychain interoperability fixture "
                                               + name + ": " + failure.what();
                    QTest::qFail(message.constData(), __FILE__, __LINE__);
                } catch (...) {
                    const QByteArray message = "Failed to clean up Keychain interoperability fixture " + name;
                    QTest::qFail(message.constData(), __FILE__, __LINE__);
                }
            }
        } cleanup { name };
        qInfo().noquote() << "Keychain interoperability dummy service:" << name;

        // This dynamic legacy symbol is intentionally confined to this
        // interoperability fixture so it can create the old item format.
        // Product code uses only the modern SecItem APIs.
        using LegacyAddGenericPassword = OSStatus (*)(
            void*, UInt32, const char*, UInt32, const char*, UInt32, const void*, void**);
        auto legacyAdd = reinterpret_cast<LegacyAddGenericPassword>(
            dlsym(RTLD_DEFAULT, "SecKeychainAddGenericPassword"));
        QVERIFY(legacyAdd);

        const QByteArray initial = "legacy-dummy-account-data";
        QCOMPARE(legacyAdd(nullptr, UInt32(name.size()), name.constData(), 0, "",
                           UInt32(initial.size()), initial.constData(), nullptr), errSecSuccess);
        QCOMPARE(macVaultRead(name), initial);

        const QByteArray updated = "updated-dummy-account-data";
        macVaultWrite(name, updated);
        QCOMPARE(macVaultRead(name), updated);

        macVaultWrite(name, {});
        QCOMPARE(macVaultRead(name), QByteArray());

        const QByteArray modern = "modern-dummy-account-data";
        macVaultWrite(name, modern);
        QCOMPARE(macVaultRead(name), modern);
        macVaultWrite(name, {});
        QCOMPARE(macVaultRead(name), QByteArray());
        macVaultWrite(name, {});
        QCOMPARE(macVaultRead(name), QByteArray());
    }
#endif
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
