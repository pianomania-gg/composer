/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "../pmmeiexport.h"

#include "engraving/compat/midi/compatmidirender.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/score.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/tremolotwochord.h"
#include "log.h"

#include "meiexporter.h"

namespace mu::iex::mei {
bool pmValidateTremoloPartnerLinks(const mu::engraving::Score* score)
{
    if (!score) {
        return false;
    }
    std::vector<const mu::engraving::Chord*> chords;
    // Chord::scanElements calls tremoloChordType, which asserts on malformed partner links.
    for (const auto* segment = score->firstSegment(mu::engraving::SegmentType::ChordRest);
         segment; segment = segment->next1(mu::engraving::SegmentType::ChordRest)) {
        for (const auto* item : segment->elist()) {
            if (item && item->isChord()) {
                chords.push_back(mu::engraving::toChord(item));
            }
        }
    }
    while (!chords.empty()) {
        const auto* chord = chords.back();
        chords.pop_back();
        for (const auto* grace : chord->graceNotes()) {
            chords.push_back(grace);
        }
        const auto* tremolo = chord->tremoloTwoChord();
        if (!tremolo) {
            continue;
        }
        const auto* first = tremolo->chord1();
        const auto* second = tremolo->chord2();
        if (!first || !second || first == second || first->score() != score || second->score() != score
            || (chord != first && chord != second)
            || first->tremoloTwoChord() != tremolo || second->tremoloTwoChord() != tremolo) {
            LOGE() << "TREM_PARTNER_ANCHOR_MISSING: tremolo=" << tremolo->eid();
            return false;
        }
    }
    return true;
}

bool pmWriteMeiToString(mu::engraving::Score* score, bool exportLayout, std::string& out,
                        std::vector<PmMeiNoteRecord>* noteRecords)
{
    if (!pmValidateTremoloPartnerLinks(score)) {
        return false;
    }
    // Canonical note indices must follow the same authored playback events as
    // coordinated MIDI, including after-graces scheduled inside their parent
    // chord duration.
    mu::engraving::CompatMidiRender::createPlayEvents(score);

    MeiExporter exporter(score);
    const bool previousExportLayout = exporter.configuration()->meiExportLayout();
    const bool previousUseMuseScoreIds = exporter.configuration()->meiUseMuseScoreIds();
    exporter.configuration()->setMeiExportLayout(exportLayout);
    exporter.configuration()->setMeiUseMuseScoreIds(true);
    exporter.setRequirePrecomputedPianomaniaIndices(true);
    exporter.setIncludeExportDate(false);
    const bool ok = exporter.write(out);
    exporter.configuration()->setMeiExportLayout(previousExportLayout);
    exporter.configuration()->setMeiUseMuseScoreIds(previousUseMuseScoreIds);
    if (!ok) {
        return false;
    }

    if (noteRecords) {
        noteRecords->clear();
        noteRecords->reserve(exporter.pianomaniaNoteRecords().size());
        for (const MeiExporter::PianomaniaNoteRecord& record : exporter.pianomaniaNoteRecords()) {
            noteRecords->push_back({ record.note, record.scoreEventId, record.chordId,
                                     record.measureId, record.beat, record.idx });
        }
    }
    return true;
}
}
