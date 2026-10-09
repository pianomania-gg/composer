// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <QByteArray>
#include <QString>
#include <optional>

class QWidget;
namespace mu::project::composer {
struct BackgroundSelection { QByteArray bytes; QString fileName; };
struct ExportSelection { BackgroundSelection background; QString destination; };
BackgroundSelection readBackground(const QString& path);
std::optional<ExportSelection> chooseExport(QWidget* parent, const QString& suggestedPath);
}
