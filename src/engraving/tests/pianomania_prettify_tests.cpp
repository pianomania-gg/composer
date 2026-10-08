/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

#include <gtest/gtest.h>

#include <array>
#include <algorithm>
#include <cmath>
#include <limits>
#include <set>
#include <vector>

#include "engraving/dom/bracketItem.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/chordrest.h"
#include "engraving/editing/editdata.h"
#include "engraving/dom/engravingitem.h"
#include "engraving/dom/beam.h"
#include "engraving/dom/fingering.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/page.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/score.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/spanner.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stem.h"
#include "engraving/dom/stafftext.h"
#include "engraving/dom/system.h"
#include "engraving/dom/tempo.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/tuplet.h"
#include "engraving/editing/undo.h"
#include "engraving/pm/pmlayout.h"
#include "engraving/pm/pmprettify.h"
#include "engraving/types/types.h"

#include "utils/scorerw.h"

using namespace mu::engraving;

namespace {

constexpr std::array<Grip, 4> PRETTIFY_GRIPS = {
    Grip::START, Grip::BEZIER1, Grip::BEZIER2, Grip::END
};

constexpr std::array<Pid, 4> PRETTIFY_SLUR_PROPERTIES = {
    Pid::SLUR_UOFF1, Pid::SLUR_UOFF2, Pid::SLUR_UOFF3, Pid::SLUR_UOFF4
};

struct SlurSnapshotEntry
{
    Slur* slur = nullptr;
    size_t segmentIndex = 0;
    std::array<PointF, 4> offsets;
    std::array<PropertyFlags, 4> offsetFlags;
    PointF offset;
    PropertyFlags offsetFlagsGeneral = PropertyFlags::STYLED;
    bool autoplace = true;
    PropertyFlags autoplaceFlags = PropertyFlags::STYLED;
    PointF endPointOff1;
    PointF endPointOff2;
    double extraHeight = 0.0;
    OffsetChange offsetChanged = OffsetChange::NONE;
    PointF changedPos;
    double spatium = 1.0;
};

struct FingeringSnapshotEntry
{
    Fingering* fingering = nullptr;
    PointF offset;
    PropertyFlags offsetFlags = PropertyFlags::STYLED;
    double minDistance = 0.0;
    PropertyFlags minDistanceFlags = PropertyFlags::STYLED;
    PlacementV placement = PlacementV::ABOVE;
    PropertyFlags placementFlags = PropertyFlags::STYLED;
    bool autoplace = true;
    PropertyFlags autoplaceFlags = PropertyFlags::STYLED;
    OffsetChange offsetChanged = OffsetChange::NONE;
    PointF changedPos;
    double spatium = 1.0;
};

struct PrettifySnapshot
{
    std::vector<SlurSnapshotEntry> slurSegments;
    std::vector<FingeringSnapshotEntry> fingerings;
};

using StructuralAssignment = std::vector<std::pair<int, int> >;

struct TempoSnapshotEntry
{
    String xmlText;
    String plainText;
    bool followText = false;
    bool visible = true;
};

struct StaffTextStyleSnapshotEntry
{
    String plainText;
    TextStyleType textStyleType = TextStyleType::DEFAULT;
    PropertyFlags textStyleFlags = PropertyFlags::STYLED;
    PropertyFlags fontFaceFlags = PropertyFlags::STYLED;
    PropertyFlags fontStyleFlags = PropertyFlags::STYLED;
    PropertyFlags fontSizeFlags = PropertyFlags::STYLED;
};

void collectFingerings(void* data, EngravingItem* item)
{
    if (!item || !item->isFingering()) {
        return;
    }

    auto* snapshot = static_cast<PrettifySnapshot*>(data);
    Fingering* fingering = toFingering(item);
    snapshot->fingerings.push_back(FingeringSnapshotEntry {
        fingering,
        fingering->offset(),
        fingering->propertyFlags(Pid::OFFSET),
        fingering->minDistance().val(),
        fingering->propertyFlags(Pid::MIN_DISTANCE),
        fingering->placement(),
        fingering->propertyFlags(Pid::PLACEMENT),
        fingering->autoplace(),
        fingering->propertyFlags(Pid::AUTOPLACE),
        fingering->ldata()->offsetChanged(),
        fingering->ldata()->autoplace.changedPos,
        std::max(1.0, fingering->spatium())
    });
}

PrettifySnapshot capturePrettifySnapshot(Score* score)
{
    PrettifySnapshot snapshot;
    if (!score) {
        return snapshot;
    }

    for (const auto& pair : score->spanner()) {
        Spanner* spanner = pair.second;
        if (!spanner || !spanner->isSlur()) {
            continue;
        }

        Slur* slur = toSlur(spanner);
        for (size_t i = 0; i < slur->nsegments(); ++i) {
            SlurSegment* segment = slur->segmentAt(static_cast<int>(i));
            if (!segment) {
                continue;
            }

            SlurSnapshotEntry entry;
            entry.slur = slur;
            entry.segmentIndex = i;
            for (size_t gripIndex = 0; gripIndex < PRETTIFY_GRIPS.size(); ++gripIndex) {
                const Grip grip = PRETTIFY_GRIPS[gripIndex];
                const Pid property = PRETTIFY_SLUR_PROPERTIES[gripIndex];
                entry.offsets[gripIndex] = segment->ups(grip).off;
                entry.offsetFlags[gripIndex] = segment->propertyFlags(property);
            }
            entry.offset = segment->offset();
            entry.offsetFlagsGeneral = segment->propertyFlags(Pid::OFFSET);
            entry.autoplace = segment->autoplace();
            entry.autoplaceFlags = segment->propertyFlags(Pid::AUTOPLACE);
            entry.endPointOff1 = segment->endPointOff1();
            entry.endPointOff2 = segment->endPointOff2();
            entry.extraHeight = segment->extraHeight();
            entry.offsetChanged = segment->ldata()->offsetChanged();
            entry.changedPos = segment->ldata()->autoplace.changedPos;
            entry.spatium = std::max(1.0, segment->spatium());
            snapshot.slurSegments.push_back(entry);
        }
    }

    score->scanElements([&snapshot](mu::engraving::EngravingItem* item) { collectFingerings(&snapshot, item); });
    return snapshot;
}

StructuralAssignment captureStructuralAssignment(Score* score)
{
    StructuralAssignment assignment;
    const Page* currentPage = nullptr;
    int pageIndex = -1;
    for (const System* system : score->systems()) {
        const Measure* firstMeasure = system ? system->firstMeasure() : nullptr;
        if (!firstMeasure) {
            continue;
        }
        const Page* page = system->page();
        if (page != currentPage) {
            currentPage = page;
            ++pageIndex;
        }
        assignment.emplace_back(pageIndex, firstMeasure->tick().ticks());
    }
    return assignment;
}

bool pointNear(const PointF& a, const PointF& b, double tolerance)
{
    return std::hypot(a.x() - b.x(), a.y() - b.y()) <= tolerance;
}

double staffYInSystem(const System* system, staff_idx_t staffIdx)
{
    if (!system || staffIdx == muse::nidx || staffIdx >= system->staves().size()) {
        return 0.0;
    }

    return system->staff(staffIdx)->y();
}

RectF fingeringSystemRect(const Fingering* fingering)
{
    const Note* note = fingering ? fingering->note() : nullptr;
    const Chord* chord = note ? note->chord() : nullptr;
    const Segment* segment = chord ? chord->segment() : nullptr;
    const Measure* measure = segment ? segment->measure() : nullptr;
    if (!fingering || !note || !chord || !segment || !measure || !fingering->ldata()) {
        return RectF();
    }

    return fingering->ldata()->bbox().translated(PointF(0.0, staffYInSystem(measure->system(), chord->vStaffIdx()))
                                                 + fingering->pos() + note->pos() + chord->pos() + segment->pos()
                                                 + measure->pos());
}

RectF noteSystemRect(const Note* note)
{
    const Chord* chord = note ? note->chord() : nullptr;
    const Segment* segment = chord ? chord->segment() : nullptr;
    const Measure* measure = segment ? segment->measure() : nullptr;
    if (!note || !chord || !segment || !measure) {
        return RectF();
    }

    return note->ldata()->bbox().translated(PointF(0.0, staffYInSystem(measure->system(), chord->vStaffIdx()))
                                            + note->pos() + chord->pos() + segment->pos() + measure->pos());
}

double fingeringNoteheadDistance(const Fingering* fingering)
{
    const RectF fingeringRect = fingeringSystemRect(fingering);
    const RectF noteRect = noteSystemRect(fingering ? fingering->note() : nullptr);
    if (fingeringRect.isNull() || noteRect.isNull()) {
        return 0.0;
    }

    const bool above = fingering->placement() == PlacementV::ABOVE;
    const double distance = above ? noteRect.top() - fingeringRect.bottom()
                            : fingeringRect.top() - noteRect.bottom();
    return std::max(0.0, distance);
}

RectF restSystemRect(const Rest* rest)
{
    const Segment* segment = rest ? rest->segment() : nullptr;
    const Measure* measure = segment ? segment->measure() : nullptr;
    if (!rest || !segment || !measure) {
        return RectF();
    }

    return rest->shape().bbox().translated(PointF(0.0, staffYInSystem(measure->system(), rest->vStaffIdx()))
                                           + rest->pos() + segment->pos() + measure->pos() + rest->staffOffset());
}

RectF hairpinSegmentSystemRect(const SpannerSegment* segment)
{
    if (!segment || !segment->ldata()) {
        return RectF();
    }

    return segment->ldata()->bbox().translated(segment->pos()
                                               + PointF(0.0, staffYInSystem(segment->system(), segment->vStaffIdx())));
}

std::vector<HairpinSegment*> collectLineHairpinSegments(Score* score)
{
    std::vector<HairpinSegment*> segments;
    if (!score) {
        return segments;
    }

    for (const auto& pair : score->spanner()) {
        Spanner* spanner = pair.second;
        if (!spanner || !spanner->isHairpin()) {
            continue;
        }

        Hairpin* hairpin = toHairpin(spanner);
        if (!hairpin->isLineType()) {
            continue;
        }
        for (size_t i = 0; i < hairpin->nsegments(); ++i) {
            HairpinSegment* segment = toHairpinSegment(hairpin->segmentAt(static_cast<int>(i)));
            if (segment) {
                segments.push_back(segment);
            }
        }
    }

    return segments;
}

std::vector<RectF> collectChordAndFingeringRects(Score* score, staff_idx_t staffIdx)
{
    std::vector<RectF> rects;
    std::set<const Beam*> seenBeams;
    if (!score) {
        return rects;
    }

    for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        System* system = measure->system();
        for (Segment* segment = measure->first(); segment; segment = segment->next()) {
            if (!segment->isChordRestType()) {
                continue;
            }
            for (EngravingItem* item : segment->elist()) {
                if (!item || !item->isChord() || item->vStaffIdx() != staffIdx) {
                    continue;
                }

                Chord* chord = toChord(item);
                for (Note* note : chord->notes()) {
                    const RectF noteRect = noteSystemRect(note);
                    if (!noteRect.isNull()) {
                        rects.push_back(noteRect);
                    }
                    for (EngravingItem* noteItem : note->el()) {
                        if (noteItem && noteItem->isFingering()) {
                            const RectF fingeringRect = fingeringSystemRect(toFingering(noteItem));
                            if (!fingeringRect.isNull()) {
                                rects.push_back(fingeringRect);
                            }
                        }
                    }
                }

                const Stem* stem = chord->stem();
                if (stem && stem->visible() && stem->ldata() && !stem->ldata()->isSkipDraw()) {
                    const RectF stemRect = stem->ldata()->bbox().translated(
                        PointF(0.0, staffYInSystem(system, staffIdx))
                        + stem->pos() + chord->pos() + segment->pos() + measure->pos());
                    if (!stemRect.isNull()) {
                        rects.push_back(stemRect);
                    }
                }

                const Beam* beam = chord->beam();
                if (beam && beam->visible() && beam->ldata() && !beam->ldata()->isSkipDraw() && seenBeams.insert(beam).second) {
                    const RectF beamRect = beam->ldata()->bbox();
                    if (!beamRect.isNull()) {
                        rects.push_back(beamRect);
                    }
                }
            }
        }
    }

    return rects;
}

RectF tupletNumberSystemRect(const Tuplet* tuplet)
{
    const Text* number = tuplet ? tuplet->number() : nullptr;
    if (!number) {
        return RectF();
    }

    return number->pageBoundingRect();
}

bool rectsOverlap(const RectF& a, const RectF& b)
{
    return !a.isNull() && !b.isNull()
           && a.left() < b.right() && a.right() > b.left()
           && a.top() < b.bottom() && a.bottom() > b.top();
}

bool slurSnapshotsEquivalent(const std::vector<SlurSnapshotEntry>& a, const std::vector<SlurSnapshotEntry>& b)
{
    if (a.size() != b.size()) {
        return false;
    }

    for (size_t i = 0; i < a.size(); ++i) {
        const SlurSnapshotEntry& left = a[i];
        const SlurSnapshotEntry& right = b[i];
        const double tolerance = 0.02 * left.spatium;
        if (left.slur != right.slur
            || left.segmentIndex != right.segmentIndex
            || left.offsetFlagsGeneral != right.offsetFlagsGeneral
            || left.autoplace != right.autoplace
            || left.autoplaceFlags != right.autoplaceFlags
            || left.offsetChanged != right.offsetChanged
            || std::abs(left.extraHeight - right.extraHeight) > 0.02
            || !pointNear(left.offset, right.offset, tolerance)) {
            return false;
        }
        if (!pointNear(left.endPointOff1, right.endPointOff1, tolerance)
            || !pointNear(left.endPointOff2, right.endPointOff2, tolerance)
            || !pointNear(left.changedPos, right.changedPos, tolerance)) {
            return false;
        }
        for (size_t gripIndex = 0; gripIndex < PRETTIFY_GRIPS.size(); ++gripIndex) {
            if (left.offsetFlags[gripIndex] != right.offsetFlags[gripIndex]
                || !pointNear(left.offsets[gripIndex], right.offsets[gripIndex], tolerance)) {
                return false;
            }
        }
    }

    return true;
}

bool fingeringSnapshotsEquivalent(const std::vector<FingeringSnapshotEntry>& a, const std::vector<FingeringSnapshotEntry>& b)
{
    if (a.size() != b.size()) {
        return false;
    }

    for (size_t i = 0; i < a.size(); ++i) {
        const FingeringSnapshotEntry& left = a[i];
        const FingeringSnapshotEntry& right = b[i];
        const double tolerance = 0.02 * left.spatium;
        if (left.fingering != right.fingering
            || left.offsetFlags != right.offsetFlags
            || left.minDistanceFlags != right.minDistanceFlags
            || left.placement != right.placement
            || left.placementFlags != right.placementFlags
            || left.autoplace != right.autoplace
            || left.autoplaceFlags != right.autoplaceFlags
            || left.offsetChanged != right.offsetChanged
            || !pointNear(left.offset, right.offset, tolerance)
            || !pointNear(left.changedPos, right.changedPos, tolerance)
            || std::abs(left.minDistance - right.minDistance) > 0.02) {
            return false;
        }
    }

    return true;
}

bool snapshotsEquivalent(const PrettifySnapshot& a, const PrettifySnapshot& b)
{
    return slurSnapshotsEquivalent(a.slurSegments, b.slurSegments)
           && fingeringSnapshotsEquivalent(a.fingerings, b.fingerings);
}

std::vector<Fingering*> collectFingeringsByText(Score* score, const String& text)
{
    std::vector<Fingering*> fingerings;
    const PrettifySnapshot snapshot = capturePrettifySnapshot(score);
    for (const FingeringSnapshotEntry& entry : snapshot.fingerings) {
        if (entry.fingering && entry.fingering->plainText() == text) {
            fingerings.push_back(entry.fingering);
        }
    }
    return fingerings;
}

std::vector<Tuplet*> collectTuplets(Score* score)
{
    std::vector<Tuplet*> tuplets;
    std::set<Tuplet*> seen;
    if (!score) {
        return tuplets;
    }

    for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (Segment* segment = measure->first(); segment; segment = segment->next()) {
            if (!segment->isChordRestType()) {
                continue;
            }
            for (EngravingItem* item : segment->elist()) {
                if (!item || !item->isChordRest()) {
                    continue;
                }
                for (Tuplet* tuplet = toChordRest(item)->tuplet(); tuplet; tuplet = tuplet->tuplet()) {
                    if (seen.insert(tuplet).second) {
                        tuplets.push_back(tuplet);
                    }
                }
            }
        }
    }
    return tuplets;
}

std::vector<Rest*> collectVisibleRests(Score* score)
{
    std::vector<Rest*> rests;
    if (!score) {
        return rests;
    }

    for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (Segment* segment = measure->first(); segment; segment = segment->next()) {
            if (!segment->isChordRestType()) {
                continue;
            }
            for (EngravingItem* item : segment->elist()) {
                if (item && item->isRest() && item->visible() && !toRest(item)->isGap()) {
                    rests.push_back(toRest(item));
                }
            }
        }
    }
    return rests;
}

size_t countBraceBracketsSpanning(const Score* score, staff_idx_t startStaffIdx, size_t span)
{
    if (!score) {
        return 0;
    }

    size_t count = 0;
    for (const Staff* staff : score->staves()) {
        if (!staff) {
            continue;
        }

        for (const BracketItem* bracket : staff->brackets()) {
            if (bracket
                && staff->idx() == startStaffIdx
                && bracket->bracketType() == BracketType::BRACE
                && bracket->bracketSpan() == span) {
                ++count;
            }
        }
    }

    return count;
}

std::vector<TempoText*> collectTempoTexts(Score* score)
{
    std::vector<TempoText*> tempoTexts;
    for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (Segment* segment = measure->first(); segment; segment = segment->next()) {
            for (EngravingItem* item : segment->annotations()) {
                if (item && item->isTempoText()) {
                    tempoTexts.push_back(toTempoText(item));
                }
            }
        }
    }
    return tempoTexts;
}

std::vector<StaffText*> collectStaffTexts(Score* score)
{
    std::vector<StaffText*> staffTexts;
    for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (Segment* segment = measure->first(); segment; segment = segment->next()) {
            for (EngravingItem* item : segment->annotations()) {
                if (item && item->isStaffText()) {
                    staffTexts.push_back(toStaffText(item));
                }
            }
        }
    }
    return staffTexts;
}

std::vector<TempoSnapshotEntry> captureTempoSnapshot(Score* score)
{
    std::vector<TempoSnapshotEntry> snapshot;
    for (TempoText* tempoText : collectTempoTexts(score)) {
        snapshot.push_back(TempoSnapshotEntry {
            tempoText->xmlText(),
            tempoText->plainText(),
            tempoText->followText(),
            tempoText->visible()
        });
    }
    return snapshot;
}

bool tempoSnapshotsEquivalent(const std::vector<TempoSnapshotEntry>& a, const std::vector<TempoSnapshotEntry>& b)
{
    if (a.size() != b.size()) {
        return false;
    }

    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i].xmlText != b[i].xmlText
            || a[i].plainText != b[i].plainText
            || a[i].followText != b[i].followText
            || a[i].visible != b[i].visible) {
            return false;
        }
    }

    return true;
}

std::vector<StaffTextStyleSnapshotEntry> captureStaffTextStyleSnapshot(Score* score)
{
    std::vector<StaffTextStyleSnapshotEntry> snapshot;
    for (StaffText* staffText : collectStaffTexts(score)) {
        snapshot.push_back(StaffTextStyleSnapshotEntry {
            staffText->plainText(),
            staffText->textStyleType(),
            staffText->propertyFlags(Pid::TEXT_STYLE),
            staffText->propertyFlags(Pid::FONT_FACE),
            staffText->propertyFlags(Pid::FONT_STYLE),
            staffText->propertyFlags(Pid::FONT_SIZE)
        });
    }
    return snapshot;
}

bool staffTextStyleSnapshotsEquivalent(const std::vector<StaffTextStyleSnapshotEntry>& a,
                                       const std::vector<StaffTextStyleSnapshotEntry>& b)
{
    if (a.size() != b.size()) {
        return false;
    }

    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i].plainText != b[i].plainText
            || a[i].textStyleType != b[i].textStyleType
            || a[i].textStyleFlags != b[i].textStyleFlags
            || a[i].fontFaceFlags != b[i].fontFaceFlags
            || a[i].fontStyleFlags != b[i].fontStyleFlags
            || a[i].fontSizeFlags != b[i].fontSizeFlags) {
            return false;
        }
    }

    return true;
}

SlurSegment* firstSlurSegment(Score* score)
{
    if (!score) {
        return nullptr;
    }

    for (const auto& pair : score->spanner()) {
        Spanner* spanner = pair.second;
        if (!spanner || !spanner->isSlur()) {
            continue;
        }

        Slur* slur = toSlur(spanner);
        if (slur->nsegments() == 0) {
            continue;
        }

        return slur->segmentAt(0);
    }

    return nullptr;
}

void relayoutScore(Score* score)
{
    score->setLayoutAll();
    score->doLayout();
}

mu::engraving::pm::PmPrettifyResult applyPrettifyCommand(Score* score)
{
    score->startCmd(TranslatableString::untranslatable("Pianomania prettify test"));
    const mu::engraving::pm::PmPrettifyResult result = mu::engraving::pm::applyPianomaniaPrettify(score);
    score->endCmd(!result.changed || result.structuralAssignmentChanged);
    relayoutScore(score);
    return result;
}

} // namespace

class Engraving_PianomaniaPrettifyTests : public ::testing::Test
{
};

// Test value: Detects a slur crossing the upper accidental of an authored turn
// when that accidental is stored as staff text on a time-tick segment.
TEST_F(Engraving_PianomaniaPrettifyTests, slurClearsOrnamentAccidentalStaffText)
{
    for (const String& symbol : { String(u"\u266d"), String(u"\u266e"), String(u"\u266f") }) {
        SCOPED_TRACE(symbol.toStdString());
        MasterScore* score = ScoreRW::readScore(u"pianomania_prettify_data/ornament-accidental-slur.mscx");
        ASSERT_TRUE(score);
        StaffText* accidental = nullptr;
        for (StaffText* text : collectStaffTexts(score)) {
            if (text->plainText() == u"\u266d") {
                accidental = text;
                break;
            }
        }
        ASSERT_TRUE(accidental);
        accidental->setXmlText(symbol);
        const StructuralAssignment originalStructure = captureStructuralAssignment(score);
        mu::engraving::pm::applyPianomaniaAutoLayout(score);
        applyPrettifyCommand(score);
        const Segment* anchor = toSegment(accidental->parentItem());
        const Measure* measure = anchor->measure();
        const System* system = measure->system();
        const RectF accidentalRect = accidental->ldata()->bbox().translated(
            accidental->pos() + anchor->pos() + measure->pos()
            + PointF(0.0, system->staff(accidental->vStaffIdx())->y()));
        ASSERT_FALSE(accidentalRect.isNull());

        SlurSegment* phrase = nullptr;
        for (const auto& pair : score->spanner()) {
            Spanner* spanner = pair.second;
            if (!spanner || !spanner->isSlur()) {
                continue;
            }
            Slur* slur = toSlur(spanner);
            if (slur->up() && slur->staffIdx() == accidental->staffIdx()
                && slur->tick() < accidental->tick() && slur->tick2() > accidental->tick()) {
                for (SpannerSegment* segment : slur->spannerSegments()) {
                    if (segment->system() == system) {
                        phrase = toSlurSegment(segment);
                        break;
                    }
                }
            }
        }
        ASSERT_TRUE(phrase);
        std::array<PointF, 4> points;
        for (size_t i = 0; i < PRETTIFY_GRIPS.size(); ++i) {
            points[i] = phrase->ups(PRETTIFY_GRIPS[i]).pos() + phrase->pos()
                        + PointF(0.0, system->staff(phrase->vStaffIdx())->y());
        }
        double minimumClearance = std::numeric_limits<double>::infinity();
        for (int i = 0; i <= 2000; ++i) {
            const double t = static_cast<double>(i) / 2000.0;
            const double u = 1.0 - t;
            const PointF point = points[0] * (u * u * u) + points[1] * (3.0 * u * u * t)
                                 + points[2] * (3.0 * u * t * t) + points[3] * (t * t * t);
            if (point.x() >= accidentalRect.left() && point.x() <= accidentalRect.right()) {
                minimumClearance = std::min(minimumClearance, accidentalRect.top() - point.y());
            }
        }
        ASSERT_TRUE(std::isfinite(minimumClearance));
        EXPECT_GE(minimumClearance, 0.1 * accidental->spatium());
        const PrettifySnapshot first = capturePrettifySnapshot(score);
        EXPECT_FALSE(applyPrettifyCommand(score).changed);
        EXPECT_TRUE(snapshotsEquivalent(first, capturePrettifySnapshot(score)));
        EXPECT_EQ(originalStructure, captureStructuralAssignment(score));
        delete score;
    }
}

// Test value: Keeps expression words, mixed text and hidden accidental glyphs
// out of the notation obstacle path without relying on the glyph classifier.
TEST_F(Engraving_PianomaniaPrettifyTests, ignoredStaffTextDoesNotShapePhraseSlur)
{
    for (int scenario = 0; scenario < 3; ++scenario) {
        SCOPED_TRACE(scenario);
        std::array<std::vector<PointF>, 2> curves;
        for (int variant = 0; variant < 2; ++variant) {
            MasterScore* score = ScoreRW::readScore(u"pianomania_prettify_data/ornament-accidental-slur.mscx");
            ASSERT_TRUE(score);
            StaffText* accidental = nullptr;
            for (StaffText* text : collectStaffTexts(score)) {
                if (text->plainText() == u"\u266d") {
                    accidental = text;
                    break;
                }
            }
            ASSERT_TRUE(accidental);
            if (variant == 0) {
                if (scenario == 0) {
                    accidental->setXmlText(u"dolce");
                } else if (scenario == 1) {
                    accidental->setXmlText(u"dolce \u266d");
                } else {
                    accidental->setVisible(false);
                }
            } else {
                score->removeElement(accidental);
                delete accidental;
            }
            mu::engraving::pm::applyPianomaniaAutoLayout(score);
            applyPrettifyCommand(score);
            for (const auto& pair : score->spanner()) {
                if (!pair.second || !pair.second->isSlur()) {
                    continue;
                }
                for (SpannerSegment* segment : toSlur(pair.second)->spannerSegments()) {
                    for (Grip grip : PRETTIFY_GRIPS) {
                        curves[variant].push_back(toSlurSegment(segment)->ups(grip).pos());
                    }
                }
            }
            delete score;
        }
        ASSERT_EQ(curves[0].size(), curves[1].size());
        ASSERT_FALSE(curves[0].empty());
        for (size_t i = 0; i < curves[0].size(); ++i) {
            EXPECT_TRUE(pointNear(curves[0][i], curves[1][i], 0.01));
        }
    }
}

TEST_F(Engraving_PianomaniaPrettifyTests, prettifyIsIdempotentAndUndoable)
{
    MasterScore* score = ScoreRW::readScore(u"pianomania_prettify_data/prettify-idempotency.mscx");
    ASSERT_TRUE(score);
    relayoutScore(score);

    const PrettifySnapshot originalSnapshot = capturePrettifySnapshot(score);
    const StructuralAssignment originalStructure = captureStructuralAssignment(score);

    const mu::engraving::pm::PmPrettifyResult firstResult = applyPrettifyCommand(score);
    EXPECT_TRUE(firstResult.changed);
    EXPECT_FALSE(firstResult.structuralAssignmentChanged);
    EXPECT_GE(firstResult.normalizedManualSlurs, 1);
    EXPECT_GE(firstResult.normalizedManualFingerings, 1);

    const PrettifySnapshot firstSnapshot = capturePrettifySnapshot(score);
    const StructuralAssignment firstStructure = captureStructuralAssignment(score);
    EXPECT_FALSE(slurSnapshotsEquivalent(originalSnapshot.slurSegments, firstSnapshot.slurSegments));
    EXPECT_FALSE(fingeringSnapshotsEquivalent(originalSnapshot.fingerings, firstSnapshot.fingerings));
    EXPECT_EQ(originalStructure, firstStructure);

    const mu::engraving::pm::PmPrettifyResult secondResult = applyPrettifyCommand(score);
    EXPECT_FALSE(secondResult.changed);
    EXPECT_FALSE(secondResult.structuralAssignmentChanged);
    EXPECT_TRUE(snapshotsEquivalent(firstSnapshot, capturePrettifySnapshot(score)));
    EXPECT_EQ(firstStructure, captureStructuralAssignment(score));

    for (int cycle = 0; cycle < 5; ++cycle) {
        SCOPED_TRACE(cycle);

        EditData undoEditData;
        score->undoStack()->undo(&undoEditData);
        relayoutScore(score);

        EXPECT_TRUE(snapshotsEquivalent(originalSnapshot, capturePrettifySnapshot(score)));
        EXPECT_EQ(originalStructure, captureStructuralAssignment(score));

        EditData redoEditData;
        score->undoStack()->redo(&redoEditData);
        relayoutScore(score);

        EXPECT_TRUE(snapshotsEquivalent(firstSnapshot, capturePrettifySnapshot(score)));
        EXPECT_EQ(firstStructure, captureStructuralAssignment(score));
    }

    delete score;
}

TEST_F(Engraving_PianomaniaPrettifyTests, fingeringClearsVisibleRestObstacle)
{
    MasterScore* score = ScoreRW::readScore(u"pianomania_prettify_data/fingering-rest-obstacle.mscx");
    ASSERT_TRUE(score);
    relayoutScore(score);

    const mu::engraving::pm::PmPrettifyResult result = applyPrettifyCommand(score);
    EXPECT_TRUE(result.changed);

    const std::vector<Fingering*> fingerings = collectFingeringsByText(score, u"1");
    ASSERT_EQ(fingerings.size(), 1);
    const std::vector<Rest*> rests = collectVisibleRests(score);
    ASSERT_FALSE(rests.empty());

    const RectF fingeringRect = fingeringSystemRect(fingerings.front());
    for (const Rest* rest : rests) {
        EXPECT_FALSE(rectsOverlap(fingeringRect, restSystemRect(rest)));
    }

    delete score;
}

TEST_F(Engraving_PianomaniaPrettifyTests, sparseCrossVoiceFingeringKeepsNoteOwnership)
{
    MasterScore* score = ScoreRW::readScore(u"pianomania_prettify_data/fingering-cross-voice-ownership.mscx");
    ASSERT_TRUE(score);
    relayoutScore(score);
    const StructuralAssignment originalStructure = captureStructuralAssignment(score);
    const PrettifySnapshot original = capturePrettifySnapshot(score);

    const auto result = applyPrettifyCommand(score);
    EXPECT_TRUE(result.changed);
    EXPECT_FALSE(result.structuralAssignmentChanged);
    const auto fingerings = collectFingeringsByText(score, u"4");
    const auto ownerDigit = std::find_if(fingerings.begin(), fingerings.end(), [](const Fingering* fingering) {
        return fingering->note()->pitch() == 55;
    });
    ASSERT_NE(ownerDigit, fingerings.end());
    const Fingering* fingering = *ownerDigit;
    const Note* owner = fingering->note();
    const Segment* segment = owner->chord()->segment();
    const Note* other = nullptr;
    for (const EngravingItem* item : segment->elist()) {
        if (item && item->isChord() && item->vStaffIdx() == owner->chord()->vStaffIdx()) {
            for (const Note* note : toChord(item)->notes()) {
                if (note->pitch() == 59) {
                    other = note;
                }
            }
        }
    }
    ASSERT_TRUE(other);
    auto assertOwnership = [&]() {
        EXPECT_EQ(fingering->note(), owner);
        const RectF digitRect = fingeringSystemRect(fingering);
        const RectF ownerRect = noteSystemRect(owner);
        const RectF otherRect = noteSystemRect(other);
        const double clearance = 0.15 * fingering->spatium();
        EXPECT_FALSE(rectsOverlap(digitRect.adjusted(-clearance, -clearance, clearance, clearance), otherRect));
        EXPECT_FALSE(rectsOverlap(digitRect.adjusted(-clearance, -clearance, clearance, clearance), ownerRect));
        // An isolated digit above B would visually assign it to B. It must
        // remain closer in pitch height to its G, alongside or below G.
        EXPECT_LT(std::abs(digitRect.center().y() - ownerRect.center().y()),
                  std::abs(digitRect.center().y() - otherRect.center().y()));
        EXPECT_EQ(originalStructure, captureStructuralAssignment(score));
    };
    assertOwnership();
    const PrettifySnapshot prettified = capturePrettifySnapshot(score);
    const auto repeated = applyPrettifyCommand(score);
    EXPECT_FALSE(repeated.changed);
    assertOwnership();
    EditData editData;
    score->undoStack()->undo(&editData);
    relayoutScore(score);
    EXPECT_TRUE(snapshotsEquivalent(original, capturePrettifySnapshot(score)));
    score->undoStack()->redo(&editData);
    relayoutScore(score);
    EXPECT_TRUE(snapshotsEquivalent(prettified, capturePrettifySnapshot(score)));
    assertOwnership();
    delete score;
}

TEST_F(Engraving_PianomaniaPrettifyTests, sparseChordFingeringsKeepOwnershipOnEitherSide)
{
    MasterScore* score = ScoreRW::readScore(u"pianomania_prettify_data/fingering-chord-ownership.mscx");
    ASSERT_TRUE(score);
    relayoutScore(score);
    const StructuralAssignment structure = captureStructuralAssignment(score);
    const auto result = applyPrettifyCommand(score);
    EXPECT_FALSE(result.structuralAssignmentChanged);
    for (const String& text : { String(u"4"), String(u"1") }) {
        const auto fingerings = collectFingeringsByText(score, text);
        ASSERT_EQ(fingerings.size(), 1);
        const Fingering* fingering = fingerings.front();
        const Note* owner = fingering->note();
        const RectF digitRect = fingeringSystemRect(fingering);
        const RectF ownerRect = noteSystemRect(owner);
        const double clearance = 0.15 * fingering->spatium();
        for (const Note* note : owner->chord()->notes()) {
            const RectF noteRect = noteSystemRect(note);
            EXPECT_FALSE(rectsOverlap(digitRect.adjusted(-clearance, -clearance, clearance, clearance), noteRect));
            if (note != owner) {
                EXPECT_LT(std::abs(digitRect.center().y() - ownerRect.center().y()),
                          std::abs(digitRect.center().y() - noteRect.center().y()));
            }
        }
    }
    EXPECT_EQ(structure, captureStructuralAssignment(score));
    delete score;
}

TEST_F(Engraving_PianomaniaPrettifyTests, tupletBlockedFingeringFlipsToClearNoteheadSide)
{
    MasterScore* score = ScoreRW::readScore(u"pianomania_prettify_data/fingering-tuplet-obstacle.mscx");
    ASSERT_TRUE(score);
    relayoutScore(score);

    const mu::engraving::pm::PmPrettifyResult result = applyPrettifyCommand(score);
    EXPECT_TRUE(result.changed);

    std::vector<Fingering*> fingerings;
    for (const String& text : { String(u"1"), String(u"2"), String(u"3") }) {
        std::vector<Fingering*> matching = collectFingeringsByText(score, text);
        ASSERT_EQ(matching.size(), 1);
        fingerings.push_back(matching.front());
    }
    const std::vector<Tuplet*> tuplets = collectTuplets(score);
    ASSERT_EQ(tuplets.size(), 1);

    RectF fingeringRect;
    for (Fingering* fingering : fingerings) {
        EXPECT_EQ(fingering->placement(), PlacementV::ABOVE);
        fingeringRect.unite(fingering->pageBoundingRect());
    }

    const RectF tupletRect = tupletNumberSystemRect(tuplets.front());
    ASSERT_FALSE(fingeringRect.isNull());
    ASSERT_FALSE(tupletRect.isNull());
    EXPECT_LT(fingeringRect.bottom(), tupletRect.top());
    EXPECT_FALSE(rectsOverlap(fingeringRect, tupletRect));

    delete score;
}

TEST_F(Engraving_PianomaniaPrettifyTests, fingeringPlacementStaysWithinNoteheadCap)
{
    MasterScore* score = ScoreRW::readScore(u"pianomania_prettify_data/fingering-notehead-cap.mscx");
    ASSERT_TRUE(score);
    relayoutScore(score);

    const mu::engraving::pm::PmPrettifyResult result = applyPrettifyCommand(score);
    EXPECT_TRUE(result.changed);

    constexpr double capSp = 3.0;
    for (const String& text : { String(u"1"), String(u"2"), String(u"3"), String(u"4") }) {
        const std::vector<Fingering*> matching = collectFingeringsByText(score, text);
        ASSERT_EQ(matching.size(), 1);
        const Fingering* fingering = matching.front();
        EXPECT_LE(fingeringNoteheadDistance(fingering), capSp * std::max(1.0, fingering->spatium()) + 0.05)
            << text.toStdString();
    }

    delete score;
}

TEST_F(Engraving_PianomaniaPrettifyTests, textHairpinClearsNotationAndFingerings)
{
    MasterScore* score = ScoreRW::readScore(u"pianomania_prettify_data/text-hairpin-notation-collision.mscx");
    ASSERT_TRUE(score);
    relayoutScore(score);

    std::vector<HairpinSegment*> hairpinSegments = collectLineHairpinSegments(score);
    ASSERT_EQ(hairpinSegments.size(), 1);
    HairpinSegment* hairpinSegment = hairpinSegments.front();
    ASSERT_EQ(hairpinSegment->hairpin()->hairpinType(), HairpinType::CRESC_LINE);

    const staff_idx_t staffIdx = hairpinSegment->vStaffIdx();
    std::vector<RectF> obstacles = collectChordAndFingeringRects(score, staffIdx);
    ASSERT_FALSE(obstacles.empty());

    const RectF beforeRect = hairpinSegmentSystemRect(hairpinSegment);
    bool initiallyOverlaps = false;
    RectF firstObstacle;
    RectF allObstacles;
    for (const RectF& obstacle : obstacles) {
        if (firstObstacle.isNull()) {
            firstObstacle = obstacle;
        }
        allObstacles.unite(obstacle);
        initiallyOverlaps = initiallyOverlaps || rectsOverlap(beforeRect, obstacle);
    }
    ASSERT_TRUE(initiallyOverlaps)
        << "hairpin before left=" << beforeRect.left() << " right=" << beforeRect.right()
        << " top=" << beforeRect.top() << " bottom=" << beforeRect.bottom()
        << " first obstacle left=" << firstObstacle.left() << " right=" << firstObstacle.right()
        << " top=" << firstObstacle.top() << " bottom=" << firstObstacle.bottom()
        << " all obstacles top=" << allObstacles.top() << " bottom=" << allObstacles.bottom();

    const mu::engraving::pm::PmPrettifyResult result = applyPrettifyCommand(score);
    EXPECT_TRUE(result.changed);

    hairpinSegments = collectLineHairpinSegments(score);
    ASSERT_EQ(hairpinSegments.size(), 1);
    hairpinSegment = hairpinSegments.front();
    obstacles = collectChordAndFingeringRects(score, hairpinSegment->vStaffIdx());
    const RectF afterRect = hairpinSegmentSystemRect(hairpinSegment);
    EXPECT_GT(afterRect.top(), beforeRect.top());
    for (const RectF& obstacle : obstacles) {
        EXPECT_FALSE(rectsOverlap(afterRect, obstacle))
            << "hairpin after left=" << afterRect.left() << " right=" << afterRect.right()
            << " top=" << afterRect.top() << " bottom=" << afterRect.bottom()
            << " obstacle left=" << obstacle.left() << " right=" << obstacle.right()
            << " top=" << obstacle.top() << " bottom=" << obstacle.bottom();
    }

    delete score;
}

TEST_F(Engraving_PianomaniaPrettifyTests, autoLayoutAddsMissingKeyboardGrandStaffBraceOnce)
{
    MasterScore* score = ScoreRW::readScore(u"pianomania_prettify_data/two-keyboard-parts-no-brace.mscx");
    ASSERT_TRUE(score);
    ASSERT_EQ(score->nstaves(), 2);
    EXPECT_EQ(countBraceBracketsSpanning(score, 0, 2), 0);

    mu::engraving::pm::applyPianomaniaAutoLayout(score);
    EXPECT_EQ(countBraceBracketsSpanning(score, 0, 2), 1);

    mu::engraving::pm::applyPianomaniaAutoLayout(score);
    EXPECT_EQ(countBraceBracketsSpanning(score, 0, 2), 1);

    delete score;
}

TEST_F(Engraving_PianomaniaPrettifyTests, autoLayoutHidesMetronomeTempoIndicatorsPreservingPlaybackTempo)
{
    MasterScore* score = ScoreRW::readScore(u"pianomania_prettify_data/tempo-indicators.mscx");
    ASSERT_TRUE(score);

    const std::vector<TempoText*> tempoTexts = collectTempoTexts(score);
    ASSERT_EQ(tempoTexts.size(), 4);
    const std::vector<StaffText*> staffTexts = collectStaffTexts(score);
    ASSERT_EQ(staffTexts.size(), 1);
    EXPECT_EQ(staffTexts.front()->plainText(), u"Allegretto");

    const BeatsPerSecond expressiveTempo = score->tempomap()->tempo(0);
    const BeatsPerSecond metronomeOnlyTempo = score->tempomap()->tempo(1920);
    const BeatsPerSecond expressiveRangeTempo = score->tempomap()->tempo(3840);
    const BeatsPerSecond circaMetronomeOnlyTempo = score->tempomap()->tempo(5760);

    mu::engraving::pm::applyPianomaniaAutoLayout(score);

    EXPECT_EQ(tempoTexts[0]->plainText(), u"Allegro");
    EXPECT_EQ(tempoTexts[0]->xmlText(), u"Allegro");
    EXPECT_FALSE(tempoTexts[0]->followText());
    EXPECT_TRUE(tempoTexts[0]->visible());
    EXPECT_FALSE(tempoTexts[1]->followText());
    EXPECT_FALSE(tempoTexts[1]->visible());
    EXPECT_TRUE(tempoTexts[1]->xmlText().isEmpty());
    EXPECT_EQ(tempoTexts[2]->plainText(), u"Allegro");
    EXPECT_EQ(tempoTexts[2]->xmlText(), u"Allegro");
    EXPECT_FALSE(tempoTexts[2]->followText());
    EXPECT_TRUE(tempoTexts[2]->visible());
    EXPECT_FALSE(tempoTexts[3]->followText());
    EXPECT_FALSE(tempoTexts[3]->visible());
    EXPECT_TRUE(tempoTexts[3]->xmlText().isEmpty());
    EXPECT_EQ(staffTexts.front()->plainText(), u"Allegretto");
    EXPECT_TRUE(muse::RealIsEqual(score->tempomap()->tempo(0).val, expressiveTempo.val));
    EXPECT_TRUE(muse::RealIsEqual(score->tempomap()->tempo(1920).val, metronomeOnlyTempo.val));
    EXPECT_TRUE(muse::RealIsEqual(score->tempomap()->tempo(3840).val, expressiveRangeTempo.val));
    EXPECT_TRUE(muse::RealIsEqual(score->tempomap()->tempo(5760).val, circaMetronomeOnlyTempo.val));

    const std::vector<TempoSnapshotEntry> firstSnapshot = captureTempoSnapshot(score);
    mu::engraving::pm::applyPianomaniaAutoLayout(score);
    EXPECT_TRUE(tempoSnapshotsEquivalent(firstSnapshot, captureTempoSnapshot(score)));
    EXPECT_TRUE(muse::RealIsEqual(score->tempomap()->tempo(0).val, expressiveTempo.val));
    EXPECT_TRUE(muse::RealIsEqual(score->tempomap()->tempo(1920).val, metronomeOnlyTempo.val));
    EXPECT_TRUE(muse::RealIsEqual(score->tempomap()->tempo(3840).val, expressiveRangeTempo.val));
    EXPECT_TRUE(muse::RealIsEqual(score->tempomap()->tempo(5760).val, circaMetronomeOnlyTempo.val));

    delete score;
}

TEST_F(Engraving_PianomaniaPrettifyTests, autoLayoutNormalizesExpressionStaffTextOnly)
{
    MasterScore* score = ScoreRW::readScore(u"pianomania_prettify_data/expression-staff-text.mscx");
    ASSERT_TRUE(score);

    const std::vector<StaffText*> staffTexts = collectStaffTexts(score);
    ASSERT_EQ(staffTexts.size(), 2);
    ASSERT_EQ(staffTexts[0]->plainText(), u"dolce");
    ASSERT_EQ(staffTexts[1]->plainText(), u"Allegretto");
    EXPECT_EQ(staffTexts[0]->textStyleType(), TextStyleType::STAFF);
    EXPECT_EQ(staffTexts[1]->textStyleType(), TextStyleType::STAFF);
    EXPECT_EQ(staffTexts[0]->propertyFlags(Pid::FONT_FACE), PropertyFlags::UNSTYLED);
    EXPECT_EQ(staffTexts[0]->propertyFlags(Pid::FONT_STYLE), PropertyFlags::UNSTYLED);
    EXPECT_EQ(staffTexts[0]->propertyFlags(Pid::FONT_SIZE), PropertyFlags::UNSTYLED);

    mu::engraving::pm::applyPianomaniaAutoLayout(score);

    EXPECT_EQ(staffTexts[0]->plainText(), u"dolce");
    EXPECT_EQ(staffTexts[0]->textStyleType(), TextStyleType::EXPRESSION);
    EXPECT_EQ(staffTexts[0]->propertyFlags(Pid::FONT_FACE), PropertyFlags::STYLED);
    EXPECT_EQ(staffTexts[0]->propertyFlags(Pid::FONT_STYLE), PropertyFlags::STYLED);
    EXPECT_EQ(staffTexts[0]->propertyFlags(Pid::FONT_SIZE), PropertyFlags::STYLED);

    EXPECT_EQ(staffTexts[1]->plainText(), u"Allegretto");
    EXPECT_EQ(staffTexts[1]->textStyleType(), TextStyleType::STAFF);

    const std::vector<StaffTextStyleSnapshotEntry> firstSnapshot = captureStaffTextStyleSnapshot(score);
    mu::engraving::pm::applyPianomaniaAutoLayout(score);
    EXPECT_TRUE(staffTextStyleSnapshotsEquivalent(firstSnapshot, captureStaffTextStyleSnapshot(score)));

    delete score;
}

TEST_F(Engraving_PianomaniaPrettifyTests, slurEndpointOffsetUndoRedoClearsStaleEndpointCarryover)
{
    MasterScore* score = ScoreRW::readScore(u"pianomania_prettify_data/prettify-idempotency.mscx");
    ASSERT_TRUE(score);
    relayoutScore(score);

    SlurSegment* segment = firstSlurSegment(score);
    ASSERT_TRUE(segment);

    const PointF originalStartOffset = segment->ups(Grip::START).off;
    const double spatium = std::max(1.0, segment->spatium());
    const PointF staleEndpointCarryover(0.41 * spatium, 0.29 * spatium);
    const PointF changedStartOffset = originalStartOffset + PointF(0.7 * spatium, -0.2 * spatium);

    segment->setEndPointOff1(staleEndpointCarryover);

    score->startCmd(TranslatableString::untranslatable("slur endpoint offset carryover test"));
    segment->undoChangeProperty(Pid::SLUR_UOFF1, changedStartOffset, segment->propertyFlags(Pid::SLUR_UOFF1));
    score->endCmd(false);
    relayoutScore(score);

    EXPECT_TRUE(pointNear(segment->ups(Grip::START).off, changedStartOffset, 0.02 * spatium));
    EXPECT_TRUE(pointNear(segment->endPointOff1(), PointF(), 0.02 * spatium));

    EditData undoEditData;
    score->undoStack()->undo(&undoEditData);
    relayoutScore(score);
    EXPECT_TRUE(pointNear(segment->ups(Grip::START).off, originalStartOffset, 0.02 * spatium));

    segment->setEndPointOff1(staleEndpointCarryover);

    EditData redoEditData;
    score->undoStack()->redo(&redoEditData);
    relayoutScore(score);
    EXPECT_TRUE(pointNear(segment->ups(Grip::START).off, changedStartOffset, 0.02 * spatium));
    EXPECT_TRUE(pointNear(segment->endPointOff1(), PointF(), 0.02 * spatium));

    delete score;
}
