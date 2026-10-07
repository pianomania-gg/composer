// SPDX-License-Identifier: GPL-3.0-only
#include "../internal/composerpackage.h"
#include "../internal/composersession.h"
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <iostream>
#include <stdexcept>

using namespace mu::project::composer;
void require(bool condition) { if (!condition) throw std::runtime_error("Contract failed"); }
template<typename F> void rejects(F action) { bool rejected = false; try { action(); } catch (const std::exception&) { rejected = true; } require(rejected); }
int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    try {
        // Firebase's documented custom-token response deliberately has no localId.
        QJsonObject exchange {{"idToken", "id-token"}, {"refreshToken", "refresh-token"}, {"expiresIn", "3600"}};
        QJsonObject account {{"users", QJsonArray {QJsonObject {{"localId", "owner"}, {"email", "owner@example.test"}}}}};
        auto signedIn = sessionFromFirebaseResponses(exchange, account, "owner", "api-key");
        require(signedIn["uid"] == "owner" && signedIn["email"] == "owner@example.test");
        require(signedIn["refreshToken"] == "refresh-token");
        rejects([&] { sessionFromFirebaseResponses(exchange, account, "other", "api-key"); });
        rejects([&] { sessionFromFirebaseResponses(exchange, {}, "owner", "api-key"); });
        auto missingToken = exchange; missingToken.remove("idToken");
        rejects([&] { sessionFromFirebaseResponses(missingToken, account, "owner", "api-key"); });
        QVector<Section> sections {{1, "song.mei", "<mei>contract</mei>"}, {2, "song.mid", QByteArray::fromHex("4d546864000000060001000101e0")}, {4, "manifest.json", "{}"}};
        QString hash = meiHash(sections);
        auto payload = QJsonDocument(QJsonObject {{"v", 1}, {"uids", QJsonArray {"owner"}}, {"fileHash", hash}}).toJson(QJsonDocument::Compact);
        QByteArray license = QJsonDocument(QJsonObject {{"payload", QString::fromLatin1(payload.toBase64())}, {"sig", QString::fromLatin1(QByteArray(256, 'x').toBase64())}}).toJson(QJsonDocument::Compact);
        validateLicense(license, "owner", hash);
        rejects([&] { validateLicense(license, "another-owner", hash); });
        rejects([&] { validateLicense(license, "owner", QString(64, '0')); });
        rejects([&] { validateLicense(license, "*", hash); });
        rejects([&] { encode(sections); });
        sections.append({6, "license.json", license});
        QByteArray encoded = encode(sections);
        require(encoded.left(6) == QByteArray::fromHex("504df0010100"));
        require(encoded != encode(sections));
        auto duplicate = sections; duplicate.append(sections[0]);
        rejects([&] { encode(duplicate); });
        auto empty = sections; empty[1].bytes.clear();
        rejects([&] { encode(empty); });
        auto longName = sections; longName[1].name = QString(256, 'x');
        rejects([&] { encode(longName); });
        if (argc > 1) { QFile output(QString::fromLocal8Bit(argv[1])); require(output.open(QIODevice::WriteOnly)); require(output.write(encoded) == encoded.size()); }
        std::cout << "Composer package contracts passed\n";
        return 0;
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
