// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <QList>
#include <QString>
#include <optional>

class QWidget;
namespace mu::project::composer {
struct SongCredit { QString role; QString name; };
struct SongDetails {
    QString title;
    QList<SongCredit> credits;
    std::optional<int> difficultyRating;
};
inline constexpr auto DifficultyRatingTag = "pianomaniaDifficultyRating";
inline constexpr auto SongDetailsVersionTag = "pianomaniaSongDetailsVersion";
std::optional<SongDetails> chooseSongDetails(QWidget* parent, const SongDetails& details);
}
