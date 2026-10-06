// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <QByteArray>
#include <QString>
#include <QVector>

namespace mu::project::composer {
struct Section { quint8 type; QString name; QByteArray bytes; };
QByteArray encode(const QVector<Section>& sections);
QString meiHash(const QVector<Section>& sections);
void validateLicense(const QByteArray& envelope, const QString& uid, const QString& hash);
}
