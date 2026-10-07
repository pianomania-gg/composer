// SPDX-License-Identifier: GPL-3.0-only
#include "composersession.h"
#include <QCryptographicHash>
#include <QDesktopServices>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProgressDialog>
#include <QPushButton>
#include <QRandomGenerator>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QUrlQuery>
#include <stdexcept>
#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <wincred.h>
#elif defined(Q_OS_MACOS)
#include <Security/Security.h>
#endif

namespace mu::project::composer {
namespace {
QString service() { return QStringLiteral(PIANOMANIA_COMPOSER_SERVICE_URL); }
QString environment() { return QStringLiteral(PIANOMANIA_COMPOSER_PROFILE); }
QString vaultName() { return "PianomaniaComposer/" + environment() + "/" + QCryptographicHash::hash(service().toUtf8(), QCryptographicHash::Sha256).toHex(); }
QJsonObject session;
bool loaded = false;
bool selectAccountOnNextSignIn = false;

QByteArray vaultRead() {
#ifdef Q_OS_WIN
    PCREDENTIALW credential = nullptr;
    const auto target = vaultName().toStdWString();
    if (!CredReadW(target.c_str(), CRED_TYPE_GENERIC, 0, &credential)) {
        if (GetLastError() == ERROR_NOT_FOUND) return {};
        throw std::runtime_error("Could not read Windows Credential Manager.");
    }
    QByteArray bytes(reinterpret_cast<char*>(credential->CredentialBlob), int(credential->CredentialBlobSize));
    CredFree(credential);
    return bytes;
#elif defined(Q_OS_MACOS)
    QByteArray name = vaultName().toUtf8();
    void* data = nullptr; UInt32 size = 0;
    OSStatus status = SecKeychainFindGenericPassword(nullptr, UInt32(name.size()), name.data(), 0, "", &size, &data, nullptr);
    if (status == errSecItemNotFound) return {};
    if (status != errSecSuccess) throw std::runtime_error("Could not read macOS Keychain.");
    QByteArray bytes(static_cast<char*>(data), int(size));
    SecKeychainItemFreeContent(nullptr, data);
    return bytes;
#else
    throw std::runtime_error("Secure Composer sign-in requires Windows or macOS.");
#endif
}
void vaultWrite(const QByteArray& bytes) {
#ifdef Q_OS_WIN
    auto target = vaultName().toStdWString();
    if (bytes.isEmpty()) {
        if (!CredDeleteW(target.c_str(), CRED_TYPE_GENERIC, 0) && GetLastError() != ERROR_NOT_FOUND)
            throw std::runtime_error("Could not remove the saved Pianomania account.");
        return;
    }
    CREDENTIALW credential {};
    credential.Type = CRED_TYPE_GENERIC; credential.TargetName = target.data();
    credential.CredentialBlob = reinterpret_cast<LPBYTE>(const_cast<char*>(bytes.constData()));
    credential.CredentialBlobSize = DWORD(bytes.size()); credential.Persist = CRED_PERSIST_LOCAL_MACHINE;
    if (!CredWriteW(&credential, 0)) throw std::runtime_error("Could not save the account in Windows Credential Manager.");
#elif defined(Q_OS_MACOS)
    QByteArray name = vaultName().toUtf8(); SecKeychainItemRef item = nullptr;
    OSStatus found = SecKeychainFindGenericPassword(nullptr, UInt32(name.size()), name.data(), 0, "", nullptr, nullptr, &item);
    OSStatus status = errSecSuccess;
    if (found == errSecSuccess) {
        status = bytes.isEmpty() ? SecKeychainItemDelete(item) : SecKeychainItemModifyAttributesAndData(item, nullptr, UInt32(bytes.size()), bytes.data());
        CFRelease(item);
    } else if (found == errSecItemNotFound && !bytes.isEmpty()) {
        status = SecKeychainAddGenericPassword(nullptr, UInt32(name.size()), name.data(), 0, "", UInt32(bytes.size()), bytes.data(), nullptr);
    } else if (found != errSecItemNotFound) status = found;
    if (status != errSecSuccess) throw std::runtime_error("Could not update macOS Keychain.");
#else
    throw std::runtime_error("Secure Composer sign-in requires Windows or macOS.");
#endif
}
void load() {
    if (loaded) return;
    QByteArray stored = vaultRead();
    session = QJsonDocument::fromJson(stored).object();
    if (!stored.isEmpty() && (session.value("uid").toString().isEmpty() || session.value("refreshToken").toString().isEmpty() || session.value("apiKey").toString().isEmpty()))
        throw std::runtime_error("The saved Pianomania account is invalid. Sign out and sign in again.");
    loaded = true;
}
QJsonObject post(const QUrl& url, const QByteArray& body, QWidget* parent, const QByteArray& bearer = {}, bool form = false) {
    QNetworkAccessManager manager;
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);
    request.setHeader(QNetworkRequest::ContentTypeHeader, form ? "application/x-www-form-urlencoded" : "application/json");
    if (!bearer.isEmpty()) request.setRawHeader("Authorization", "Bearer " + bearer);
    QNetworkReply* reply = manager.post(request, body);
    QProgressDialog progress("Pianomania Composer", "Cancel", 0, 0, parent);
    progress.setWindowModality(Qt::ApplicationModal); progress.setMinimumDuration(0);
    QEventLoop loop;
    QTimer timeout; timeout.setSingleShot(true); timeout.setInterval(30000);
    QObject::connect(&timeout, &QTimer::timeout, reply, &QNetworkReply::abort);
    QObject::connect(&progress, &QProgressDialog::canceled, reply, &QNetworkReply::abort);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(reply, &QNetworkReply::downloadProgress, reply, [reply](qint64 received, qint64) { if (received > 32768) reply->abort(); });
    timeout.start(); loop.exec(); progress.close();
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray bytes = reply->readAll();
    if (reply->error() != QNetworkReply::NoError || status != 200 || bytes.size() > 32768) {
        if (status == 401 || status == 400) throw std::runtime_error("Sign-in expired or failed. Sign out and sign in again.");
        if (status == 403) throw std::runtime_error("Composer is not enabled for this account.");
        if (status == 404) throw std::runtime_error("Composer sign-in is not enabled on this server.");
        if (status == 429) throw std::runtime_error("Too many Composer requests. Wait a minute and try again.");
        throw std::runtime_error("The Composer request failed or was cancelled. Check your connection and try again.");
    }
    QJsonDocument document = QJsonDocument::fromJson(bytes);
    if (!document.isObject()) throw std::runtime_error("The Composer server returned an invalid response.");
    return document.object();
}
QString randomParameter() {
    QByteArray bytes;
    for (int i = 0; i < 8; ++i) { quint32 value = QRandomGenerator::system()->generate(); bytes.append(reinterpret_cast<const char*>(&value), 4); }
    return QString::fromLatin1(bytes.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
}
void signIn(QWidget* parent, bool switchAccount = false) {
    QTcpServer server;
    if (!server.listen(QHostAddress::LocalHost, 0)) throw std::runtime_error("Could not open the local sign-in callback.");
    QString redirect = QString("http://127.0.0.1:%1/composer/callback").arg(server.serverPort());
    QString state = randomParameter(), verifier = randomParameter(), code;
    QString challenge = QString::fromLatin1(QCryptographicHash::hash(verifier.toLatin1(), QCryptographicHash::Sha256).toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
    QUrl url(service() + "/composer/authorize");
    // Firebase authorizes localhost for development browser sign-in. The callback
    // listener and native HTTP requests still use the literal loopback address.
    if (url.scheme() == "http" && url.host() == "127.0.0.1") url.setHost("localhost");
    QUrlQuery query; query.addQueryItem("challenge", challenge); query.addQueryItem("redirectUri", redirect);
    query.addQueryItem("state", state); query.addQueryItem("environment", environment()); url.setQuery(query);
    if (switchAccount || selectAccountOnNextSignIn) { query.addQueryItem("switchAccount", "1"); url.setQuery(query); }
    QProgressDialog progress("Sign in to your Pianomania account.", "Cancel", 0, 0, parent);
    progress.setWindowModality(Qt::ApplicationModal); progress.setMinimumDuration(0);
    QEventLoop loop; QTimer timeout; timeout.setSingleShot(true); timeout.setInterval(180000);
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(&progress, &QProgressDialog::canceled, &loop, &QEventLoop::quit);
    QObject::connect(&server, &QTcpServer::newConnection, &loop, [&]() {
        while (server.hasPendingConnections()) {
            QTcpSocket* socket = server.nextPendingConnection();
            socket->setParent(&server); socket->setReadBufferSize(8193);
            QObject::connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
            QTimer::singleShot(5000, socket, &QTcpSocket::abort);
            QObject::connect(socket, &QTcpSocket::readyRead, &loop, [&, socket]() {
                QByteArray data = socket->property("request").toByteArray() + socket->readAll();
                if (data.size() > 8192) { socket->abort(); return; }
                if (!data.contains("\r\n\r\n")) { socket->setProperty("request", data); return; }
                QList<QByteArray> requestLine = data.left(data.indexOf("\r\n")).split(' ');
                bool valid = requestLine.size() == 3 && requestLine[0] == "GET";
                QUrl callback = valid ? QUrl(QString::fromUtf8(requestLine[1])) : QUrl();
                QUrlQuery result(callback);
                QString received = result.queryItemValue("code");
                valid = valid && callback.path() == "/composer/callback" && callback.host().isEmpty()
                    && result.queryItems().size() == 2 && result.queryItemValue("state") == state
                    && received.size() == 43 && !progress.wasCanceled();
                if (valid && code.isEmpty()) code = received;
                socket->write(valid ? "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nCache-Control: no-store\r\nConnection: close\r\n\r\nSigned in. Return to Pianomania Composer."
                    : "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\nInvalid sign-in callback.");
                socket->disconnectFromHost();
                if (valid) loop.quit();
            });
        }
    });
    if (!QDesktopServices::openUrl(url)) throw std::runtime_error("Could not open your browser.");
    timeout.start(); loop.exec(); server.close(); progress.close();
    if (code.isEmpty()) throw std::runtime_error("Sign-in was cancelled or timed out.");
    auto bridge = post(QUrl(service() + "/api/composer/token"), QJsonDocument(QJsonObject {
        {"code", code}, {"verifier", verifier}, {"redirectUri", redirect}, {"environment", environment()}
    }).toJson(QJsonDocument::Compact), parent);
    QString key = bridge["apiKey"].toString(), uid = bridge["uid"].toString();
    if (key.isEmpty() || uid.isEmpty() || bridge["customToken"].toString().isEmpty()) throw std::runtime_error("Invalid sign-in response.");
    QUrl exchange("https://identitytoolkit.googleapis.com/v1/accounts:signInWithCustomToken");
    QUrlQuery parameters; parameters.addQueryItem("key", key); exchange.setQuery(parameters);
    auto result = post(exchange, QJsonDocument(QJsonObject {{"token", bridge["customToken"]}, {"returnSecureToken", true}}).toJson(QJsonDocument::Compact), parent);
    if (result["idToken"].toString().isEmpty() || result["refreshToken"].toString().isEmpty())
        throw std::runtime_error("We couldn't complete sign-in. Return to Composer and try again.");
    // Custom-token exchange returns tokens, not localId. Resolve the identity
    // through Firebase's authenticated account lookup before saving credentials.
    QUrl lookup("https://identitytoolkit.googleapis.com/v1/accounts:lookup");
    lookup.setQuery(parameters);
    auto account = post(lookup, QJsonDocument(QJsonObject {{"idToken", result["idToken"]}}).toJson(QJsonDocument::Compact), parent);
    QJsonObject next = sessionFromFirebaseResponses(result, account, uid, key);
    vaultWrite(QJsonDocument(next).toJson(QJsonDocument::Compact)); session = next; loaded = true;
    selectAccountOnNextSignIn = false;
    accountState()->refresh();
}
QByteArray token(QWidget* parent) {
    load();
    if (session.isEmpty()) signIn(parent);
    QUrl url("https://securetoken.googleapis.com/v1/token"); QUrlQuery query; query.addQueryItem("key", session.value("apiKey").toString()); url.setQuery(query);
    QUrlQuery form; form.addQueryItem("grant_type", "refresh_token"); form.addQueryItem("refresh_token", session.value("refreshToken").toString());
    auto result = post(url, form.query(QUrl::FullyEncoded).toUtf8(), parent, {}, true);
    if (result["user_id"].toString() != session.value("uid").toString() || result["id_token"].toString().isEmpty() || result["refresh_token"].toString().isEmpty())
        throw std::runtime_error("The refreshed account did not match. Sign out and sign in again.");
    session["refreshToken"] = result["refresh_token"];
    vaultWrite(QJsonDocument(session).toJson(QJsonDocument::Compact));
    return result["id_token"].toString().toUtf8();
}
}
AccountState::AccountState(QObject* parent) : QObject(parent) { refresh(); }
void AccountState::refresh() {
    QString uid, email, error;
    try { load(); uid = session.value("uid").toString(); email = session.value("email").toString(); }
    catch (const std::exception& failure) { error = QString::fromUtf8(failure.what()); }
    if (uid == m_uid && email == m_email && error == m_error) return;
    m_uid = uid; m_email = email; m_error = error;
    emit changed();
}
AccountState* accountState() { static AccountState state; return &state; }
QJsonObject sessionFromFirebaseResponses(const QJsonObject& exchange, const QJsonObject& account,
                                        const QString& uid, const QString& apiKey) {
    auto users = account["users"].toArray();
    if (uid.isEmpty() || uid == "*" || apiKey.isEmpty() || exchange["idToken"].toString().isEmpty()
        || exchange["refreshToken"].toString().isEmpty() || users.size() != 1
        || users[0].toObject()["localId"].toString() != uid)
        throw std::runtime_error("We couldn't confirm your account. Return to Composer and sign in again.");
    return {{"uid", uid}, {"apiKey", apiKey}, {"refreshToken", exchange["refreshToken"]},
            {"email", users[0].toObject()["email"]}};
}
QString currentUid() { load(); return session.value("uid").toString(); }
QByteArray license(const QString& hash, QWidget* parent) {
    QByteArray bearer = token(parent);
    auto result = post(QUrl(service() + "/api/composer/license"), QJsonDocument(QJsonObject {{"fileHash", hash}}).toJson(QJsonDocument::Compact), parent, bearer);
    return QJsonDocument(result).toJson(QJsonDocument::Compact);
}
void showAccount(QWidget* parent) {
    try {
        load();
        if (session.isEmpty()) { signIn(parent); return; }
        QString identity = session.value("email").toString();
        if (identity.isEmpty()) identity = session.value("uid").toString();
        QDialog dialog(parent);
        dialog.setObjectName("composerAccountDialog");
        dialog.setWindowTitle("Pianomania account");
        dialog.setMinimumWidth(480);
        dialog.setStyleSheet("QLabel { font-size: 14px; } QPushButton { font-size: 14px; padding: 9px 16px; }"
                             "QPushButton#signOut { color: #ffffff; background-color: #b8323e; border: 1px solid #b8323e; border-radius: 4px; }"
                             "QPushButton#signOut:hover { background-color: #982a34; }"
                             "QPushButton#signOut:focus { border: 2px solid palette(highlight); }");
        auto layout = new QVBoxLayout(&dialog);
        layout->setContentsMargins(24, 24, 24, 24);
        layout->setSpacing(16);
        auto heading = new QLabel("Signed in as", &dialog);
        layout->addWidget(heading);
        auto email = new QLabel(identity, &dialog);
        email->setObjectName("accountEmail");
        email->setTextFormat(Qt::PlainText);
        email->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
        email->setWordWrap(true);
        email->setStyleSheet("font-size: 20px; font-weight: 600;");
        layout->addWidget(email);
        auto explanation = new QLabel("Songs you export belong to this account. Use the same account in Pianomania to play them.", &dialog);
        explanation->setWordWrap(true);
        layout->addWidget(explanation);
        auto manageAccount = new QPushButton("Manage account", &dialog);
        manageAccount->setToolTip("Open your Pianomania account in your browser");
        auto switchAccount = new QPushButton("Switch account", &dialog);
        auto accountActions = new QHBoxLayout;
        accountActions->addWidget(manageAccount);
        accountActions->addWidget(switchAccount);
        layout->addLayout(accountActions);
        auto browserNote = new QLabel("Account management opens in your browser.", &dialog);
        browserNote->setWordWrap(true);
        layout->addWidget(browserNote);
        layout->addSpacing(8);
        auto footer = new QHBoxLayout;
        auto signOut = new QPushButton("Sign out", &dialog);
        signOut->setObjectName("signOut");
        auto close = new QPushButton("Done", &dialog);
        close->setDefault(true);
        for (auto button : {manageAccount, switchAccount, signOut}) button->setAutoDefault(false);
        footer->addWidget(signOut);
        footer->addStretch();
        footer->addWidget(close);
        layout->addLayout(footer);
        QObject::connect(close, &QPushButton::clicked, &dialog, &QDialog::reject);
        QObject::connect(signOut, &QPushButton::clicked, &dialog, [&dialog] { dialog.done(1); });
        QObject::connect(switchAccount, &QPushButton::clicked, &dialog, [&dialog] { dialog.done(2); });
        QObject::connect(manageAccount, &QPushButton::clicked, &dialog, [&dialog] {
            if (!QDesktopServices::openUrl(QUrl(service() + "/beta-portal/account")))
                QMessageBox::warning(&dialog, "Pianomania account", "Could not open your browser.");
        });
        close->setFocus();
        const int action = dialog.exec();
        if (action == 1) {
            vaultWrite({}); session = {}; loaded = true; selectAccountOnNextSignIn = true;
            accountState()->refresh();
        } else if (action == 2) { signIn(parent, true); }
    } catch (const std::exception& error) { QMessageBox::warning(parent, "Pianomania Composer", QString::fromUtf8(error.what())); }
}
}
