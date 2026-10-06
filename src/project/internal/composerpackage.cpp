// SPDX-License-Identifier: GPL-3.0-only
#include "composerpackage.h"
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRandomGenerator>
#include <QSet>
#include <stdexcept>

namespace mu::project::composer {
namespace {
void append32(QByteArray& bytes, quint32 value) { for (int i = 0; i < 4; ++i) bytes.append(char(value >> (8 * i))); }
quint32 next(quint32* s) {
    quint32 t = s[3], x = s[0];
    s[3] = s[2]; s[2] = s[1]; s[1] = x;
    t ^= t << 11; t ^= t >> 8; s[0] = t ^ x ^ (x >> 19);
    return s[0];
}
}
QString meiHash(const QVector<Section>& sections) {
    for (const auto& section : sections) if (section.type == 1)
        return QString::fromLatin1(QCryptographicHash::hash(section.bytes, QCryptographicHash::Sha256).toHex());
    throw std::runtime_error("The score export has no MEI.");
}
void validateLicense(const QByteArray& envelope, const QString& uid, const QString& hash) {
    const auto object = QJsonDocument::fromJson(envelope).object();
    const auto payload = QJsonDocument::fromJson(QByteArray::fromBase64(object["payload"].toString().toLatin1())).object();
    const auto owners = payload["uids"].toArray();
    if (uid.isEmpty() || uid == "*" || payload["v"].toInt() != 1 || owners.size() != 1
        || owners[0].toString() != uid || payload["fileHash"].toString() != hash
        || QByteArray::fromBase64(object["sig"].toString().toLatin1()).size() < 256)
        throw std::runtime_error("The license does not match this account and score.");
}
QByteArray encode(const QVector<Section>& sections) {
    if (sections.size() < 4 || sections.size() > 6) throw std::runtime_error("Invalid package section count.");
    QSet<int> types;
    QSet<QString> names;
    QByteArray body;
    body.append(char(sections.size())); body.append(char(0));
    qsizetype total = 22 + 2;
    for (const auto& section : sections) {
        QByteArray name = section.name.toUtf8();
        total += 6 + name.size() + section.bytes.size();
        if (section.type < 1 || section.type > 6 || types.contains(section.type) || names.contains(section.name)
            || name.isEmpty() || name.size() > 255 || section.bytes.isEmpty() || total > 256 * 1024 * 1024)
            throw std::runtime_error("The package has invalid, duplicate, empty, or oversized sections.");
        types.insert(section.type); names.insert(section.name);
        body.append(char(section.type)); body.append(char(name.size())); body.append(name);
        append32(body, quint32(section.bytes.size()));
    }
    for (int required : {1, 2, 4, 6}) if (!types.contains(required)) throw std::runtime_error("A required export file is missing.");
    for (const auto& section : sections) body.append(section.bytes);
    QByteArray salt;
    for (int i = 0; i < 4; ++i) append32(salt, QRandomGenerator::system()->generate());
    QByteArray seed = QByteArray("pm.composer.v1::d3f1b6a07c9e4a82b5104f6e8a2c1d0") + salt;
    quint32 state[] {0x9E3779B9, 0x243F6A88, 0xB7E15162, 0xDEADBEEF};
    for (int i = 0; i < seed.size(); i += 4) {
        quint32 word = 0;
        for (int j = 0; j < 4 && i+j < seed.size(); ++j) word |= quint32(quint8(seed[i+j])) << (8*j);
        state[(i/4)&3] ^= word;
    }
    for (int i = 0; i < 16; ++i) next(state);
    for (qsizetype i = 0; i < body.size();) {
        quint32 word = next(state);
        for (int j = 0; j < 4 && i < body.size(); ++j, ++i) body[i] = char(quint8(body[i]) ^ quint8(word >> (8*j)));
    }
    return QByteArray::fromHex("504df0010100") + salt + body;
}
}
