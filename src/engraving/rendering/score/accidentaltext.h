/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef MU_ENGRAVING_RENDERING_SCORE_ACCIDENTALTEXT_H
#define MU_ENGRAVING_RENDERING_SCORE_ACCIDENTALTEXT_H

#include "dom/stafftext.h"

namespace mu::engraving::rendering::score {
// Some imported ornament accidentals are authored as single-glyph staff text.
inline bool isAccidentalStaffText(const EngravingItem* item)
{
    if (!item || !item->isStaffText()) {
        return false;
    }
    const String text = toStaffText(item)->plainText().trimmed();
    return text == u"\u266d" || text == u"\u266e" || text == u"\u266f";
}
}

#endif
