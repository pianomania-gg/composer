/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iterator>
#include <cfloat>
#include <cmath>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "io/file.h"

#include "engraving/tests/utils/scorerw.h"
#include "engraving/tests/utils/scorecomp.h"

#include "engraving/dom/masterscore.h"
#include "engraving/dom/articulation.h"
#include "engraving/dom/accidental.h"
#include "engraving/dom/excerpt.h"
#include "engraving/dom/beam.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/chordrest.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/expression.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/line.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/note.h"
#include "engraving/dom/page.h"
#include "engraving/dom/pedal.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/spanner.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftext.h"
#include "engraving/dom/system.h"
#include "engraving/pm/pmlayout.h"
#include "engraving/rendering/score/systemlayout.h"
#include "engraving/dom/volta.h"
#include "engraving/dom/tuplet.h"
#include "engraving/dom/trill.h"
#include "engraving/iengravingfont.h"
#include "engraving/types/symnames.h"

#include "modularity/ioc.h"
#include "importexport/mei/imeiconfiguration.h"
#include "importexport/mei/internal/meireader.h"
#include "importexport/mei/internal/meiwriter.h"
#include "importexport/mei/pmmeiexport.h"

using namespace mu::engraving;

static const String MEI_DIR(u"data/");

////////////////////////////////////////////////////////////////
// Set to true to re-generate the MuseScore reference test files
#define BUILD_MSCORE_REF_FILE false
////////////////////////////////////////////////////////////////

namespace mu::iex::mei {
class Mei_Tests : public ::testing::Test
{
public:
    void meiReadTest(const char* file);

    inline static bool s_generateReferenceFile = BUILD_MSCORE_REF_FILE;
};

void Mei_Tests::meiReadTest(const char* file)
{
    String fileName = String::fromUtf8(file);

    auto importFunc = [](MasterScore* score, const muse::io::path_t& path) -> Err {
        MeiReader meiReader(nullptr);
        return meiReader.import(score, path);
    };

    auto exportFunc = [](Score* score, const muse::io::path_t& path) -> Err {
        MeiWriter meiWriter;
        return meiWriter.writeScore(score, path);
    };

    // Load the .mei file
    MasterScore* score = ScoreRW::readScore(MEI_DIR + fileName + u".mei", false, importFunc);
    EXPECT_TRUE(score);

    // Flag to be turned on to generate the test reference .mscx files from the .mei
    if (s_generateReferenceFile) {
        bool res = ScoreRW::saveScore(score, ScoreRW::rootPath() + u"/" + MEI_DIR + fileName + u".mscx");
        EXPECT_TRUE(res);
        return;
    }

    // Compare with the reference MuseScore file
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, fileName + u".mscx", MEI_DIR + fileName + u".mscx"));

    // Save the .mei file for round trip testing
    bool output = ScoreRW::saveScore(score,  fileName + u".test.mei", exportFunc);
    EXPECT_TRUE(output);
    delete score;

    // Compare the mei files
    EXPECT_TRUE(ScoreComp::compareFiles(fileName + u".test.mei", ScoreRW::rootPath() + u"/" + MEI_DIR + fileName + u".mei"));
}

struct PianomaniaPrettifyFlagScope {
    bool previousPrettify = false;
    bool previousForceNormalize = false;

    PianomaniaPrettifyFlagScope(bool prettify, bool forceNormalize)
        : previousPrettify(MScore::pianomaniaPrettifySlursFingerings),
        previousForceNormalize(MScore::pianomaniaForceNormalizeSlursFingerings)
    {
        MScore::pianomaniaPrettifySlursFingerings = prettify;
        MScore::pianomaniaForceNormalizeSlursFingerings = forceNormalize;
        MScore::resetPianomaniaSlurFingeringDiagnostics();
    }

    ~PianomaniaPrettifyFlagScope()
    {
        MScore::pianomaniaPrettifySlursFingerings = previousPrettify;
        MScore::pianomaniaForceNormalizeSlursFingerings = previousForceNormalize;
    }
};

struct FingeringExportData {
    std::string tag;
    std::string text;
    std::optional<std::string> pmxy;
    std::optional<double> yOffset;
};

std::string readTestTextFile(const String& fileName)
{
    muse::io::File file(fileName);
    EXPECT_TRUE(file.open(muse::io::IODevice::ReadOnly));
    if (!file.isOpen()) {
        return std::string();
    }

    const auto data = file.readAll();
    return std::string(reinterpret_cast<const char*>(data.constData()), data.size());
}

std::optional<std::string> xmlAttributeValue(const std::string& tag, const std::string& name)
{
    const std::string needle = name + "=\"";
    size_t start = tag.find(needle);
    if (start == std::string::npos) {
        return std::nullopt;
    }

    start += needle.size();
    const size_t end = tag.find('"', start);
    if (end == std::string::npos) {
        return std::nullopt;
    }

    return tag.substr(start, end - start);
}

std::optional<double> xmlAttributeDouble(const std::string& tag, const std::string& name)
{
    const std::optional<std::string> value = xmlAttributeValue(tag, name);
    if (!value.has_value()) {
        return std::nullopt;
    }

    char* end = nullptr;
    const double parsed = std::strtod(value->c_str(), &end);
    if (end == value->c_str()) {
        return std::nullopt;
    }

    return parsed;
}

std::vector<std::string> collectStartTags(const std::string& xmlText, const std::string& elementName)
{
    std::vector<std::string> tags;
    const std::string needle = "<" + elementName;
    size_t cursor = 0;
    while ((cursor = xmlText.find(needle, cursor)) != std::string::npos) {
        const size_t end = xmlText.find('>', cursor);
        if (end == std::string::npos) {
            break;
        }

        tags.push_back(xmlText.substr(cursor, end - cursor + 1));
        cursor = end + 1;
    }

    return tags;
}

std::vector<FingeringExportData> collectFingeringExportData(const std::string& meiText)
{
    std::vector<FingeringExportData> fingerings;
    size_t cursor = 0;
    while ((cursor = meiText.find("<fing", cursor)) != std::string::npos) {
        const size_t tagEnd = meiText.find('>', cursor);
        if (tagEnd == std::string::npos) {
            break;
        }

        const size_t close = meiText.find("</fing>", tagEnd);
        if (close == std::string::npos) {
            break;
        }

        FingeringExportData data;
        data.tag = meiText.substr(cursor, tagEnd - cursor + 1);
        data.text = meiText.substr(tagEnd + 1, close - tagEnd - 1);
        data.pmxy = xmlAttributeValue(data.tag, "pm:xy");
        data.yOffset = xmlAttributeDouble(data.tag, "yOffset");
        fingerings.push_back(data);
        cursor = close + 7;
    }

    return fingerings;
}

std::vector<FingeringExportData> exportPianomaniaFingeringObstacleFixture(bool prettify, bool forceNormalize,
                                                                         const String& outputName, std::string* outputText,
                                                                         int* normalizedManualFingerings)
{
    PianomaniaPrettifyFlagScope flagScope(prettify, forceNormalize);

    auto exportFunc = [](Score* score, const muse::io::path_t& path) -> Err {
        MeiWriter meiWriter;
        return meiWriter.writeScore(score, path);
    };

    MasterScore* score = ScoreRW::readScore(MEI_DIR + u"pianomania-fingering-obstacles.mscx", false);
    EXPECT_TRUE(score);
    if (!score) {
        return {};
    }

    score->setLayoutAll();
    score->doLayout();

    const bool output = ScoreRW::saveScore(score, outputName, exportFunc);
    EXPECT_TRUE(output);
    delete score;

    if (normalizedManualFingerings) {
        *normalizedManualFingerings = MScore::pianomaniaNormalizedManualFingerings;
    }

    *outputText = readTestTextFile(outputName);
    return collectFingeringExportData(*outputText);
}

TEST_F(Mei_Tests, mei_accid_01) {
    meiReadTest("accid-01");
}

TEST_F(Mei_Tests, mei_accid_02) {
    meiReadTest("accid-02");
}

TEST_F(Mei_Tests, mei_arpeg_01) {
    meiReadTest("arpeg-01");
}

TEST_F(Mei_Tests, mei_artic_01) {
    meiReadTest("artic-01");
}

TEST_F(Mei_Tests, mei_artic_02) {
    meiReadTest("artic-02");
}

TEST_F(Mei_Tests, mei_beam_01) {
    meiReadTest("beam-01");
}

TEST_F(Mei_Tests, mei_beam_02) {
    meiReadTest("beam-02");
}

TEST_F(Mei_Tests, mei_beam_03) {
    meiReadTest("beam-03");
}

TEST_F(Mei_Tests, mei_export_beam_boundaries_ignore_omitted_hidden_chords) {
    auto exportFunc = [](Score* score, const muse::io::path_t& path) -> Err {
        MeiWriter meiWriter;
        return meiWriter.writeScore(score, path);
    };

    MasterScore* score = ScoreRW::readScore(MEI_DIR + u"pianomania-visible-beam-boundaries.mscx", false);
    ASSERT_TRUE(score);

    const String outputName = u"pianomania-visible-beam-boundaries.test.mei";
    ASSERT_TRUE(ScoreRW::saveScore(score, outputName, exportFunc));
    delete score;

    const std::string meiText = readTestTextFile(outputName);
    EXPECT_EQ(collectStartTags(meiText, "beam").size(), 2u);
    EXPECT_EQ(collectStartTags(meiText, "note").size(), 6u);
}

TEST_F(Mei_Tests, mei_export_pianomania_rubato_zone) {
    auto exportFunc = [](Score* score, const muse::io::path_t& path) -> Err {
        MeiWriter meiWriter;
        return meiWriter.writeScore(score, path);
    };

    MasterScore* score = ScoreRW::readScore(MEI_DIR + u"pianomania-rubato-zone.mscx", false);
    ASSERT_TRUE(score);

    score->setLayoutAll();
    score->doLayout();

    const String outputName = u"pianomania-rubato-zone.test.mei";
    ASSERT_TRUE(ScoreRW::saveScore(score, outputName, exportFunc));
    delete score;

    const std::string meiText = readTestTextFile(outputName);

    const size_t lineOpen = meiText.find("<line");
    ASSERT_NE(lineOpen, std::string::npos);
    const size_t lineEnd = meiText.find(">", lineOpen);
    ASSERT_NE(lineEnd, std::string::npos);
    const std::string lineTag = meiText.substr(lineOpen, lineEnd - lineOpen + 1);

    EXPECT_NE(lineTag.find("type=\"pm-rubato-zone\""), std::string::npos) << lineTag;
    EXPECT_NE(lineTag.find("pm:whole-measures=\"true\""), std::string::npos) << lineTag;
    EXPECT_NE(lineTag.find("startid=\"#"), std::string::npos) << lineTag;
    EXPECT_NE(lineTag.find("endid=\"#"), std::string::npos) << lineTag;
    EXPECT_NE(lineTag.find("pm:x1y1x2y2=\""), std::string::npos) << lineTag;
    EXPECT_NE(lineTag.find("pm:segments=\""), std::string::npos) << lineTag;
    // Exactly one zone in the fixture.
    EXPECT_EQ(meiText.find("<line", lineOpen + 1), std::string::npos);

    const std::optional<std::string> endid = xmlAttributeValue(lineTag, "endid");
    ASSERT_TRUE(endid.has_value());
    ASSERT_GT(endid->size(), 1u);
    const size_t firstMeasure = meiText.find("<measure");
    ASSERT_NE(firstMeasure, std::string::npos);
    const size_t secondMeasure = meiText.find("<measure", firstMeasure + 1);
    ASSERT_NE(secondMeasure, std::string::npos);
    const size_t secondMeasureEnd = meiText.find("</measure>", secondMeasure);
    ASSERT_NE(secondMeasureEnd, std::string::npos);
    const size_t endAnchor = meiText.find(
        "xml:id=\"" + endid->substr(1) + "\"", secondMeasure);
    EXPECT_LT(endAnchor, secondMeasureEnd);
    EXPECT_EQ(meiText.find("pname=\"c\" oct=\"6\""), std::string::npos);
}

TEST_F(Mei_Tests, mei_rubato_overlay_preserves_layout_and_export_when_hidden) {
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(MEI_DIR + u"pianomania-rubato-overlay.mscx", false));
    ASSERT_TRUE(score);
    score->setLayoutAll();
    score->doLayout();

    Spanner* zone = nullptr;
    for (const auto& interval : score->spannerMap().findOverlapping(0, score->endTick().ticks())) {
        if (interval.value && interval.value->isRubatoZone()) {
            zone = interval.value;
            break;
        }
    }
    ASSERT_TRUE(zone);
    ASSERT_FALSE(zone->spannerSegments().empty());
    for (SpannerSegment* segment : zone->spannerSegments()) {
        EXPECT_FALSE(segment->addToSkyline());
        EXPECT_TRUE(segment->collectForDrawing());
    }

    // Compare rendered notation geometry with the zone present, hidden, and removed.
    auto notationGeometry = [&]() {
        std::vector<RectF> geometry;
        for (const System* system : score->systems()) {
            geometry.push_back(system->pageBoundingRect());
            for (const SysStaff* staff : system->staves()) {
                geometry.push_back(staff->bbox());
            }
        }
        score->scanElements([&](EngravingItem* item) {
            if (item->isChord() || item->isRest() || item->isStaffText() || item->isTempoText()
                || item->isStaffLines() || item->isBarLine()) {
                geometry.push_back(item->pageBoundingRect());
            }
        });
        return geometry;
    };
    const auto withZone = notationGeometry();
    ASSERT_FALSE(withZone.empty());
    std::string shownMei;
    ASSERT_TRUE(pmWriteMeiToString(score.get(), true, shownMei));
    score->setShowRubatoZones(false);
    for (SpannerSegment* segment : zone->spannerSegments()) {
        EXPECT_FALSE(segment->collectForDrawing());
        EXPECT_FALSE(segment->isInteractionAvailable());
        EXPECT_TRUE(segment->visible());
    }
    score->setLayoutAll();
    score->doLayout();
    EXPECT_EQ(notationGeometry(), withZone);
    std::string hiddenMei;
    ASSERT_TRUE(pmWriteMeiToString(score.get(), true, hiddenMei));
    EXPECT_EQ(shownMei, hiddenMei);

    score->removeSpanner(zone);
    zone->eraseSpannerSegments();
    delete zone;
    score->setLayoutAll();
    score->doLayout();
    EXPECT_EQ(notationGeometry(), withZone);
}

TEST_F(Mei_Tests, mei_chopin_rubato_overlay_does_not_move_notation) {
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(MEI_DIR + u"chopin-op9-no1-rubato-overlay.mscz", false));
    ASSERT_TRUE(score);
    score->setLayoutAll();
    score->doLayout();
    auto notationGeometry = [&]() {
        std::vector<RectF> geometry;
        for (const System* system : score->systems()) {
            geometry.push_back(system->pageBoundingRect());
            for (const SysStaff* staff : system->staves()) {
                geometry.push_back(staff->bbox());
            }
        }
        score->scanElements([&](EngravingItem* item) {
            if (item->isChord() || item->isRest() || item->isStaffText() || item->isTempoText()
                || item->isStaffLines() || item->isBarLine()) {
                geometry.push_back(item->pageBoundingRect());
            }
        });
        return geometry;
    };
    const auto originalGeometry = notationGeometry();
    std::string originalMei;
    ASSERT_TRUE(pmWriteMeiToString(score.get(), true, originalMei));
    ASSERT_TRUE(muse::io::File::writeFile(String(u"chopin-op9-no1-original.test.mei"),
                                        muse::ByteArray(originalMei.c_str(), originalMei.size())));
    const auto tempos = collectStartTags(originalMei, "tempo");
    EXPECT_EQ(std::count_if(tempos.begin(), tempos.end(), [](const std::string& tag) {
        return xmlAttributeValue(tag, "visible") == std::optional<std::string>("false");
    }), 16);

    std::vector<Spanner*> zones;
    for (const auto& interval : score->spannerMap().findOverlapping(0, score->endTick().ticks())) {
        if (interval.value && interval.value->isRubatoZone()) {
            zones.push_back(interval.value);
        }
    }
    ASSERT_FALSE(zones.empty());
    score->setShowRubatoZones(false);
    score->setLayoutAll();
    score->doLayout();
    EXPECT_EQ(notationGeometry(), originalGeometry);
    std::string hiddenMei;
    ASSERT_TRUE(pmWriteMeiToString(score.get(), true, hiddenMei));
    EXPECT_EQ(originalMei, hiddenMei);
    ASSERT_TRUE(muse::io::File::writeFile(String(u"chopin-op9-no1-rubato-overlay.test.mei"),
                                        muse::ByteArray(hiddenMei.c_str(), hiddenMei.size())));
    for (Spanner* zone : zones) {
        score->removeSpanner(zone);
        zone->eraseSpannerSegments();
        delete zone;
    }
    for (const System* system : score->systems()) {
        for (const SpannerSegment* segment : system->spannerSegments()) {
            EXPECT_NE(segment->type(), ElementType::RUBATO_ZONE_SEGMENT);
        }
    }
    score->setLayoutAll();
    score->doLayout();
    EXPECT_EQ(notationGeometry(), originalGeometry);
}

TEST_F(Mei_Tests, mei_hidden_tempo_preserves_text_and_playback_with_visibility) {
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(MEI_DIR + u"pianomania-rubato-overlay.mscx", false));
    ASSERT_TRUE(score);
    bool foundTempo = false;
    score->scanElements([&](EngravingItem* item) {
        if (item->isTempoText()) {
            item->setVisible(false);
            foundTempo = true;
        }
    });
    ASSERT_TRUE(foundTempo);
    const String outputName = u"hidden-tempo.test.mei";
    ASSERT_TRUE(ScoreRW::saveScore(score.get(), outputName, [](Score* score, const muse::io::path_t& path) -> Err {
        MeiWriter writer;
        return writer.writeScore(score, path);
    }));
    const auto text = readTestTextFile(outputName);
    const auto tempos = collectStartTags(text, "tempo");
    ASSERT_EQ(tempos.size(), 1u);
    EXPECT_EQ(xmlAttributeValue(tempos[0], "visible"), std::optional<std::string>("false"));
    EXPECT_TRUE(xmlAttributeValue(tempos[0], "midi.bpm").has_value());
    EXPECT_NE(text.find("rallentando"), std::string::npos);
}

TEST_F(Mei_Tests, mei_export_pianomania_470_properties_survive_mscx_round_trip) {
    auto exportFunc = [](Score* score, const muse::io::path_t& path) -> Err {
        MeiWriter meiWriter;
        return meiWriter.writeScore(score, path);
    };

    MasterScore* score = ScoreRW::readScore(MEI_DIR + u"pianomania-rubato-zone.mscx", false);
    ASSERT_TRUE(score);

    const String roundTripName = u"pianomania-470-properties.roundtrip.mscx";
    ASSERT_TRUE(ScoreRW::saveScore(score, roundTripName));
    delete score;

    const std::string roundTripMscx = readTestTextFile(roundTripName);
    EXPECT_NE(roundTripMscx.find("<pianomaniaHeldNotePitchCurve version=\"3\">"), std::string::npos);
    EXPECT_NE(roundTripMscx.find(
        "<point scoreTick=\"120\" pitchCents=\"-200\" slopeCentsPerQuarter=\"-960\"/>"), std::string::npos);

    MasterScore* roundTrippedScore = ScoreRW::readScore(roundTripName, true);
    ASSERT_TRUE(roundTrippedScore);

    size_t heldNoteCount = 0;
    size_t pulsingNoteCount = 0;
    size_t quarterPulseCount = 0;
    size_t tripletPulseCount = 0;
    size_t heldPitchCurveCount = 0;
    size_t shakeNoteCount = 0;
    size_t rightHandNoteCount = 0;
    size_t rightHandRestCount = 0;
    roundTrippedScore->scanElements([&](EngravingItem* item) {
        if (item->isRest()) {
            const Rest* rest = toRest(item);
            rightHandRestCount += rest->pianomaniaHand()
                == static_cast<int>(Note::PianomaniaHand::Right) ? 1 : 0;
            return;
        }
        if (!item->isNote()) {
            return;
        }
        const Note* note = toNote(item);
        heldNoteCount += note->pianomaniaHeldNote() ? 1 : 0;
        pulsingNoteCount += note->pianomaniaHeldNotePulse()
            != Note::PianomaniaHeldNotePulse::None ? 1 : 0;
        quarterPulseCount += note->pianomaniaHeldNotePulse()
            == Note::PianomaniaHeldNotePulse::Quarter ? 1 : 0;
        tripletPulseCount += note->pianomaniaHeldNotePulseTriplet() ? 1 : 0;
        heldPitchCurveCount += note->pianomaniaHeldNotePitchCurve().empty() ? 0 : 1;
        shakeNoteCount += note->pianomaniaShakeNote() ? 1 : 0;
        rightHandNoteCount += note->pianomaniaHand() == Note::PianomaniaHand::Right ? 1 : 0;
    });
    EXPECT_EQ(heldNoteCount, 3u);
    EXPECT_EQ(pulsingNoteCount, 2u);
    EXPECT_EQ(quarterPulseCount, 1u);
    EXPECT_EQ(tripletPulseCount, 1u);
    EXPECT_EQ(heldPitchCurveCount, 1u);
    EXPECT_EQ(shakeNoteCount, 1u);
    EXPECT_EQ(rightHandNoteCount, 1u);
    EXPECT_EQ(rightHandRestCount, 1u);

    const auto spanners = roundTrippedScore->spannerMap().findOverlapping(
        0, roundTrippedScore->endTick().ticks());
    const Spanner* rubatoZone = nullptr;
    for (const auto& interval : spanners) {
        if (interval.value && interval.value->isRubatoZone()) {
            rubatoZone = interval.value;
            break;
        }
    }
    ASSERT_TRUE(rubatoZone);
    EXPECT_EQ(rubatoZone->tick().ticks(), 0);
    EXPECT_EQ(rubatoZone->tick2().ticks(), 3840);
    EXPECT_EQ(rubatoZone->track(), 0);
    EXPECT_EQ(rubatoZone->track2(), 0);

    roundTrippedScore->setLayoutAll();
    roundTrippedScore->doLayout();
    const String outputName = u"pianomania-470-properties.roundtrip.test.mei";
    ASSERT_TRUE(ScoreRW::saveScore(roundTrippedScore, outputName, exportFunc));
    delete roundTrippedScore;

    const std::string meiText = readTestTextFile(outputName);
    const std::vector<std::string> noteTags = collectStartTags(meiText, "note");
    std::vector<std::string> regularNoteTags;
    std::copy_if(noteTags.cbegin(), noteTags.cend(), std::back_inserter(regularNoteTags), [](const std::string& tag) {
        return xmlAttributeValue(tag, "pname").has_value();
    });
    ASSERT_GE(regularNoteTags.size(), 4u);
    EXPECT_EQ(xmlAttributeValue(regularNoteTags[0], "held"), "true");
    EXPECT_EQ(xmlAttributeValue(regularNoteTags[0], "heldPulse"), "quarter");
    EXPECT_EQ(xmlAttributeValue(regularNoteTags[0], "heldPulseTriplet"), "true");
    EXPECT_EQ(xmlAttributeValue(regularNoteTags[0], "pm:heldPitchCurveV3"),
              "0:0:-667;120:-200:-960;480:-1200:-1733");
    EXPECT_FALSE(xmlAttributeValue(regularNoteTags[0], "pm:heldPitchCurveV2").has_value());
    EXPECT_EQ(xmlAttributeValue(regularNoteTags[1], "held"), "true");
    EXPECT_EQ(xmlAttributeValue(regularNoteTags[1], "heldPulse"), "eighth");
    EXPECT_EQ(xmlAttributeValue(regularNoteTags[1], "heldPulseTriplet"), "false");
    EXPECT_EQ(xmlAttributeValue(regularNoteTags[2], "held"), "true");
    EXPECT_FALSE(xmlAttributeValue(regularNoteTags[2], "heldPulse").has_value());
    EXPECT_FALSE(xmlAttributeValue(regularNoteTags[2], "heldPulseTriplet").has_value());
    EXPECT_FALSE(xmlAttributeValue(regularNoteTags[3], "held").has_value());
    EXPECT_FALSE(xmlAttributeValue(regularNoteTags[3], "heldPulse").has_value());
    EXPECT_FALSE(xmlAttributeValue(regularNoteTags[3], "heldPulseTriplet").has_value());
    EXPECT_NE(meiText.find("shake=\"true\""), std::string::npos);
    EXPECT_NE(meiText.find("hand=\"right\""), std::string::npos);
    EXPECT_NE(meiText.find("type=\"pm-rubato-zone\""), std::string::npos);
    const std::vector<std::string> restTags = collectStartTags(meiText, "rest");
    EXPECT_TRUE(std::any_of(restTags.cbegin(), restTags.cend(), [](const std::string& tag) {
        return tag.find("hand=\"right\"") != std::string::npos;
    }));
}

TEST_F(Mei_Tests, mei_export_pianomania_rubato_zone_resolves_hidden_boundaries_to_one_anchor) {
    auto exportFunc = [](Score* score, const muse::io::path_t& path) -> Err {
        MeiWriter meiWriter;
        return meiWriter.writeScore(score, path);
    };

    MasterScore* score = ScoreRW::readScore(
        MEI_DIR + u"pianomania-rubato-zone-single-anchor.mscx", false);
    ASSERT_TRUE(score);

    score->setLayoutAll();
    score->doLayout();

    const String outputName = u"pianomania-rubato-zone-single-anchor.test.mei";
    ASSERT_TRUE(ScoreRW::saveScore(score, outputName, exportFunc));
    delete score;

    const std::string meiText = readTestTextFile(outputName);
    const std::vector<std::string> lines = collectStartTags(meiText, "line");
    ASSERT_EQ(lines.size(), 1u);

    const std::optional<std::string> startid = xmlAttributeValue(lines[0], "startid");
    const std::optional<std::string> endid = xmlAttributeValue(lines[0], "endid");
    ASSERT_TRUE(startid.has_value());
    ASSERT_TRUE(endid.has_value());
    EXPECT_EQ(startid, endid);
    ASSERT_GT(startid->size(), 1u);

    const std::string anchorId = "xml:id=\"" + startid->substr(1) + "\"";
    const size_t noteAnchor = meiText.find("<note " + anchorId);
    const size_t chordAnchor = meiText.find("<chord " + anchorId);
    const size_t restAnchor = meiText.find("<rest " + anchorId);
    EXPECT_TRUE(noteAnchor != std::string::npos ||
                chordAnchor != std::string::npos ||
                restAnchor != std::string::npos);
    EXPECT_EQ(meiText.find("pname=\"c\" oct=\"6\""), std::string::npos);
    EXPECT_EQ(meiText.find("pname=\"d\" oct=\"6\""), std::string::npos);
}

TEST_F(Mei_Tests, mei_export_pianomania_rubato_zone_overlap_fails) {
    auto exportFunc = [](Score* score, const muse::io::path_t& path) -> Err {
        MeiWriter meiWriter;
        return meiWriter.writeScore(score, path);
    };

    MasterScore* score = ScoreRW::readScore(MEI_DIR + u"pianomania-rubato-zone-overlap.mscx", false);
    ASSERT_TRUE(score);

    score->setLayoutAll();
    score->doLayout();

    // Overlapping zones are invalid; the coordinated export must fail.
    EXPECT_FALSE(ScoreRW::saveScore(score, u"pianomania-rubato-zone-overlap.test.mei", exportFunc));
    delete score;
}

TEST_F(Mei_Tests, mei_export_pianomania_pyro_span_free_range) {
    auto exportFunc = [](Score* score, const muse::io::path_t& path) -> Err {
        MeiWriter meiWriter;
        return meiWriter.writeScore(score, path);
    };

    MasterScore* score = ScoreRW::readScore(MEI_DIR + u"pianomania-pyro-span.mscx", false);
    ASSERT_TRUE(score);

    // The fixture span is free-range: mid-measure start (beat 2 of measure 1)
    // through beat 2 of measure 2, unlike the whole-measure rubato zones.
    const Spanner* pyroSpan = nullptr;
    auto spanners = score->spannerMap().findOverlapping(0, score->endTick().ticks());
    for (auto interval : spanners) {
        if (interval.value && interval.value->isPyroSpan()) {
            pyroSpan = interval.value;
        }
    }
    ASSERT_TRUE(pyroSpan);
    EXPECT_EQ(pyroSpan->tick().ticks(), 480);
    EXPECT_EQ(pyroSpan->tick2().ticks(), 2880);

    score->setLayoutAll();
    score->doLayout();

    const String outputName = u"pianomania-pyro-span.test.mei";
    ASSERT_TRUE(ScoreRW::saveScore(score, outputName, exportFunc));
    delete score;

    const std::string meiText = readTestTextFile(outputName);
    const std::vector<std::string> lines = collectStartTags(meiText, "line");
    ASSERT_EQ(lines.size(), 1u);
    const std::string& lineTag = lines[0];

    EXPECT_NE(lineTag.find("type=\"pm-pyro-span\""), std::string::npos) << lineTag;
    EXPECT_EQ(lineTag.find("pm:whole-measures"), std::string::npos) << lineTag;
    EXPECT_NE(lineTag.find("pm:x1y1x2y2=\""), std::string::npos) << lineTag;
    EXPECT_NE(lineTag.find("pm:segments=\""), std::string::npos) << lineTag;
    EXPECT_EQ(xmlAttributeValue(lineTag, "pm:start-measure-index"), "0");
    EXPECT_EQ(xmlAttributeValue(lineTag, "pm:start-beat"), "2.0000");
    EXPECT_EQ(xmlAttributeValue(lineTag, "pm:end-measure-index"), "1");
    EXPECT_EQ(xmlAttributeValue(lineTag, "pm:end-beat"), "3.0000");

    const std::optional<std::string> startid = xmlAttributeValue(lineTag, "startid");
    const std::optional<std::string> endid = xmlAttributeValue(lineTag, "endid");
    ASSERT_TRUE(startid.has_value());
    ASSERT_TRUE(endid.has_value());
    ASSERT_GT(startid->size(), 1u);
    ASSERT_GT(endid->size(), 1u);
    EXPECT_NE(startid, endid);

    // The start anchor is the D4 on beat 2 of measure 1.
    const size_t startAnchor = meiText.find("xml:id=\"" + startid->substr(1) + "\"");
    ASSERT_NE(startAnchor, std::string::npos);
    const size_t startAnchorEnd = meiText.find(">", startAnchor);
    const std::string startAnchorTag = meiText.substr(startAnchor, startAnchorEnd - startAnchor);
    EXPECT_NE(startAnchorTag.find("pname=\"d\""), std::string::npos) << startAnchorTag;

    // The reference anchor prefers the E4 attacked at the exact exclusive
    // end tick. The position attributes retain the semantic end independently.
    const size_t endAnchor = meiText.find("xml:id=\"" + endid->substr(1) + "\"");
    ASSERT_NE(endAnchor, std::string::npos);
    const size_t endAnchorEnd = meiText.find(">", endAnchor);
    const std::string endAnchorTag = meiText.substr(endAnchor, endAnchorEnd - endAnchor);
    EXPECT_NE(endAnchorTag.find("pname=\"e\""), std::string::npos) << endAnchorTag;
}

TEST_F(Mei_Tests, mei_export_pianomania_pyro_span_rest_boundary) {
    auto exportFunc = [](Score* score, const muse::io::path_t& path) -> Err {
        MeiWriter meiWriter;
        return meiWriter.writeScore(score, path);
    };

    MasterScore* score = ScoreRW::readScore(MEI_DIR + u"pianomania-pyro-span-rest-boundary.mscx", false);
    ASSERT_TRUE(score);

    score->setLayoutAll();
    score->doLayout();

    const String outputName = u"pianomania-pyro-span-rest-boundary.test.mei";
    ASSERT_TRUE(ScoreRW::saveScore(score, outputName, exportFunc));
    delete score;

    const std::string meiText = readTestTextFile(outputName);
    const std::vector<std::string> lines = collectStartTags(meiText, "line");
    ASSERT_EQ(lines.size(), 1u);
    const std::string& lineTag = lines[0];

    EXPECT_EQ(xmlAttributeValue(lineTag, "pm:start-measure-index"), "0");
    EXPECT_EQ(xmlAttributeValue(lineTag, "pm:start-beat"), "2.0000");
    EXPECT_EQ(xmlAttributeValue(lineTag, "pm:end-measure-index"), "0");
    EXPECT_EQ(xmlAttributeValue(lineTag, "pm:end-beat"), "5.0000");

    const std::optional<std::string> startid = xmlAttributeValue(lineTag, "startid");
    const std::optional<std::string> endid = xmlAttributeValue(lineTag, "endid");
    ASSERT_TRUE(startid.has_value());
    ASSERT_TRUE(endid.has_value());
    ASSERT_GT(startid->size(), 1u);
    ASSERT_GT(endid->size(), 1u);

    EXPECT_NE(meiText.find("<rest xml:id=\"" + startid->substr(1) + "\""), std::string::npos);
    EXPECT_NE(meiText.find("xml:id=\"" + endid->substr(1) + "\""), std::string::npos);
}

TEST_F(Mei_Tests, mei_export_pianomania_laser_span_coexists_with_rubato_zone) {
    auto exportFunc = [](Score* score, const muse::io::path_t& path) -> Err {
        MeiWriter meiWriter;
        return meiWriter.writeScore(score, path);
    };

    MasterScore* score = ScoreRW::readScore(MEI_DIR + u"pianomania-laser-span.mscx", false);
    ASSERT_TRUE(score);

    score->setLayoutAll();
    score->doLayout();

    // The laser span (measure 1) overlaps a rubato zone (measures 1-2);
    // cross-type overlap is valid and the export must succeed.
    const String outputName = u"pianomania-laser-span.test.mei";
    ASSERT_TRUE(ScoreRW::saveScore(score, outputName, exportFunc));
    delete score;

    const std::string meiText = readTestTextFile(outputName);
    const std::vector<std::string> lines = collectStartTags(meiText, "line");
    ASSERT_EQ(lines.size(), 2u);

    const bool firstIsLaser = lines[0].find("type=\"pm-laser-span\"") != std::string::npos;
    const std::string& laserTag = firstIsLaser ? lines[0] : lines[1];
    const std::string& rubatoTag = firstIsLaser ? lines[1] : lines[0];

    EXPECT_NE(laserTag.find("type=\"pm-laser-span\""), std::string::npos) << laserTag;
    EXPECT_EQ(laserTag.find("pm:whole-measures"), std::string::npos) << laserTag;
    EXPECT_NE(laserTag.find("startid=\"#"), std::string::npos) << laserTag;
    EXPECT_NE(laserTag.find("endid=\"#"), std::string::npos) << laserTag;
    EXPECT_EQ(xmlAttributeValue(laserTag, "pm:start-measure-index"), "0");
    EXPECT_EQ(xmlAttributeValue(laserTag, "pm:start-beat"), "1.0000");
    EXPECT_EQ(xmlAttributeValue(laserTag, "pm:end-measure-index"), "0");
    EXPECT_EQ(xmlAttributeValue(laserTag, "pm:end-beat"), "5.0000");

    EXPECT_NE(rubatoTag.find("type=\"pm-rubato-zone\""), std::string::npos) << rubatoTag;
    EXPECT_NE(rubatoTag.find("pm:whole-measures=\"true\""), std::string::npos) << rubatoTag;
}

TEST_F(Mei_Tests, mei_export_pianomania_pyro_span_overlap_fails) {
    auto exportFunc = [](Score* score, const muse::io::path_t& path) -> Err {
        MeiWriter meiWriter;
        return meiWriter.writeScore(score, path);
    };

    MasterScore* score = ScoreRW::readScore(MEI_DIR + u"pianomania-pyro-span-overlap.mscx", false);
    ASSERT_TRUE(score);

    score->setLayoutAll();
    score->doLayout();

    // Same-type overlapping spans are invalid; the export must fail.
    EXPECT_FALSE(ScoreRW::saveScore(score, u"pianomania-pyro-span-overlap.test.mei", exportFunc));
    delete score;
}

TEST_F(Mei_Tests, mei_export_beam_and_tuplet_boundaries_ignore_omitted_hidden_chord) {
    auto exportFunc = [](Score* score, const muse::io::path_t& path) -> Err {
        MeiWriter meiWriter;
        return meiWriter.writeScore(score, path);
    };

    MasterScore* score = ScoreRW::readScore(MEI_DIR + u"pianomania-visible-beam-tuplet-boundaries.mscx", false);
    ASSERT_TRUE(score);

    score->setLayoutAll();
    score->doLayout();

    const String outputName = u"pianomania-visible-beam-tuplet-boundaries.test.mei";
    ASSERT_TRUE(ScoreRW::saveScore(score, outputName, exportFunc));
    delete score;

    const std::string meiText = readTestTextFile(outputName);
    EXPECT_EQ(collectStartTags(meiText, "beam").size(), 1u);
    const std::vector<std::string> tupletTags = collectStartTags(meiText, "tuplet");
    ASSERT_EQ(tupletTags.size(), 1u);
    EXPECT_EQ(xmlAttributeValue(tupletTags.front(), "bracket.place"), "above");
    EXPECT_EQ(xmlAttributeValue(tupletTags.front(), "num.place"), "above");
    EXPECT_EQ(collectStartTags(meiText, "note").size(), 2u);

    const size_t beamOpen = meiText.find("<beam");
    const size_t tupletOpen = meiText.find("<tuplet", beamOpen);
    const size_t tupletClose = meiText.find("</tuplet>", tupletOpen);
    const size_t beamClose = meiText.find("</beam>", tupletClose);
    EXPECT_NE(beamOpen, std::string::npos);
    EXPECT_NE(tupletOpen, std::string::npos);
    EXPECT_NE(tupletClose, std::string::npos);
    EXPECT_NE(beamClose, std::string::npos);
}

TEST_F(Mei_Tests, mei_export_tuplets_include_complete_resolved_geometry) {
    auto exportFunc = [](Score* score, const muse::io::path_t& path) -> Err {
        MeiWriter meiWriter;
        return meiWriter.writeScore(score, path);
    };

    MasterScore* score = ScoreRW::readScore(MEI_DIR + u"tuplet-03.mscx", false);
    ASSERT_TRUE(score);
    score->setLayoutAll();
    score->doLayout();

    const String outputName = u"pianomania-tuplet-geometry.test.mei";
    ASSERT_TRUE(ScoreRW::saveScore(score, outputName, exportFunc));
    delete score;

    const std::vector<std::string> tags = collectStartTags(readTestTextFile(outputName), "tuplet");
    ASSERT_GE(tags.size(), 8u);

    bool foundAbove = false;
    bool foundBelow = false;
    bool foundVisibleBracket = false;
    bool foundHiddenBracket = false;
    bool foundNumberOnly = false;
    bool foundHiddenNumberWithBracket = false;
    bool foundRatio = false;
    bool foundHorizontalBracket = false;
    bool foundSlopedBracket = false;

    for (const std::string& tag : tags) {
        EXPECT_EQ(xmlAttributeValue(tag, "pm:tuplet-geometry-version"), "1") << tag;
        const std::optional<std::string> placement = xmlAttributeValue(tag, "pm:tuplet-placement");
        const std::optional<std::string> numberVisible = xmlAttributeValue(tag, "pm:tuplet-number-visible");
        const std::optional<std::string> bracketVisible = xmlAttributeValue(tag, "pm:tuplet-bracket-visible");
        ASSERT_TRUE(placement.has_value()) << tag;
        ASSERT_TRUE(numberVisible.has_value()) << tag;
        ASSERT_TRUE(bracketVisible.has_value()) << tag;

        foundAbove |= placement == "above";
        foundBelow |= placement == "below";
        const bool resolvedNumberVisible = numberVisible == "true";
        const bool resolvedBracketVisible = bracketVisible == "true";
        foundVisibleBracket |= resolvedBracketVisible;
        foundHiddenBracket |= !resolvedBracketVisible;
        foundNumberOnly |= resolvedNumberVisible && !resolvedBracketVisible;
        foundHiddenNumberWithBracket |= !resolvedNumberVisible && resolvedBracketVisible;
        foundRatio |= xmlAttributeValue(tag, "num.format") == "ratio";

        EXPECT_EQ(xmlAttributeValue(tag, "pm:tuplet-number-center").has_value(), resolvedNumberVisible) << tag;
        EXPECT_EQ(xmlAttributeValue(tag, "pm:tuplet-bracket-segments").has_value(), resolvedBracketVisible) << tag;
        EXPECT_EQ(xmlAttributeValue(tag, "pm:tuplet-bracket-hooks").has_value(), resolvedBracketVisible) << tag;

        if (!resolvedBracketVisible) {
            continue;
        }

        const std::string segments = *xmlAttributeValue(tag, "pm:tuplet-bracket-segments");
        const std::string hooks = *xmlAttributeValue(tag, "pm:tuplet-bracket-hooks");
        EXPECT_EQ(std::count(hooks.begin(), hooks.end(), ';'), 1) << tag;
        EXPECT_EQ(std::count(segments.begin(), segments.end(), ';'), resolvedNumberVisible ? 1 : 0) << tag;

        const size_t segmentEnd = segments.find(';');
        std::stringstream stream(segments.substr(0, segmentEnd));
        std::string coordinate;
        std::vector<double> values;
        while (std::getline(stream, coordinate, ',')) {
            values.push_back(std::strtod(coordinate.c_str(), nullptr));
        }
        ASSERT_EQ(values.size(), 4u) << tag;
        ASSERT_GT(std::abs(values[2] - values[0]), 0.000001) << tag;
        const double slope = (values[3] - values[1]) / (values[2] - values[0]);
        foundHorizontalBracket |= std::abs(slope) < 0.001;
        foundSlopedBracket |= std::abs(slope) >= 0.001;
    }

    EXPECT_TRUE(foundAbove);
    EXPECT_TRUE(foundBelow);
    EXPECT_TRUE(foundVisibleBracket);
    EXPECT_TRUE(foundHiddenBracket);
    EXPECT_TRUE(foundNumberOnly);
    EXPECT_TRUE(foundHiddenNumberWithBracket);
    EXPECT_TRUE(foundRatio);
    EXPECT_TRUE(foundHorizontalBracket);
    EXPECT_TRUE(foundSlopedBracket);
}

namespace {
struct GhostSegmentCase {
    const char16_t* fixture;
    bool (*matches)(const Spanner* spanner);
    const char* elementName;
    const char* requiredText;
};

size_t countSegments(const std::string& segments)
{
    return segments.empty() ? 0u : static_cast<size_t>(std::count(segments.begin(), segments.end(), ';')) + 1u;
}

std::vector<double> parseCoordinates(const std::string& value)
{
    std::vector<double> coordinates;
    std::stringstream stream(value);
    std::string coordinate;
    while (std::getline(stream, coordinate, ',')) {
        coordinates.push_back(std::strtod(coordinate.c_str(), nullptr));
    }
    return coordinates;
}
} // namespace

TEST_F(Mei_Tests, mei_export_omits_spanner_segments_without_a_score_page) {
    auto exportFunc = [](Score* score, const muse::io::path_t& path) -> Err {
        MeiWriter meiWriter;
        return meiWriter.writeScore(score, path);
    };

    const std::vector<GhostSegmentCase> cases = {
        { u"hairpin-01.mscx", [](const Spanner* s) { return s->isHairpin() && toHairpin(s)->isLineType(); }, "dir", "mscore-hairpin" },
        { u"octave-01.mscx", [](const Spanner* s) { return s->isOttava(); }, "octave", "" },
        { u"pianomania-rubato-zone.mscx", [](const Spanner* s) { return s->isRubatoZone(); }, "line", "pm-rubato-zone" },
        { u"pianomania-pyro-span.mscx", [](const Spanner* s) { return s->isPyroSpan(); }, "line", "pm-pyro-span" },
        { u"pianomania-laser-span.mscx", [](const Spanner* s) { return s->isLaserSpan(); }, "line", "pm-laser-span" },
    };

    for (const GhostSegmentCase& testCase : cases) {
        SCOPED_TRACE(String(testCase.fixture).toStdString());
        MasterScore* score = ScoreRW::readScore(MEI_DIR + String(testCase.fixture), false);
        ASSERT_TRUE(score);
        score->setLayoutAll();
        score->doLayout();

        SLine* line = nullptr;
        std::vector<Spanner*> others;
        for (const auto& entry : score->spannerMap().map()) {
            Spanner* spanner = entry.second;
            if (!spanner || !testCase.matches(spanner)) {
                continue;
            }
            if (!line && !spanner->segmentsEmpty()) {
                line = static_cast<SLine*>(spanner);
            } else {
                others.push_back(spanner);
            }
        }
        ASSERT_TRUE(line);
        // Keep one spanner of the kind so the exported element is unambiguous.
        for (Spanner* other : others) {
            score->removeSpanner(other);
        }
        const size_t placedSegments = line->spannerSegments().size();

        // A recycled layout system that no page holds, as left behind by an earlier layout pass.
        System* unplacedSystem = Factory::createSystem(score->dummy()->page());
        LineSegment* ghost = line->createLineSegment(unplacedSystem);
        ghost->setSystem(unplacedSystem);
        ghost->setPos(PointF(36.0, 50.0));
        ghost->setPos2(PointF(160.0, 0.0));
        line->add(ghost);
        ASSERT_EQ(line->spannerSegments().size(), placedSegments + 1);

        const String outputName = String(u"pianomania-ghost-segment-") + String(testCase.fixture) + u".test.mei";
        ASSERT_TRUE(ScoreRW::saveScore(score, outputName, exportFunc));

        const std::string meiText = readTestTextFile(outputName);
        std::vector<std::string> tags;
        for (const std::string& tag : collectStartTags(meiText, testCase.elementName)) {
            if (tag.find(testCase.requiredText) != std::string::npos) {
                tags.push_back(tag);
            }
        }
        ASSERT_EQ(tags.size(), 1u);
        const std::optional<std::string> segments = xmlAttributeValue(tags[0], "pm:segments");
        const std::optional<std::string> extent = xmlAttributeValue(tags[0], "pm:x1y1x2y2");
        ASSERT_TRUE(segments.has_value()) << tags[0];
        ASSERT_TRUE(extent.has_value()) << tags[0];
        EXPECT_EQ(countSegments(*segments), placedSegments) << tags[0];
        const size_t lastSeparator = segments->rfind(';');
        const std::string lastSegment = lastSeparator == std::string::npos ? *segments : segments->substr(lastSeparator + 1);
        // The overall extent ends where the last placed segment ends, not at the ghost.
        EXPECT_EQ(extent->substr(extent->find(',', extent->find(',') + 1)),
                  lastSegment.substr(lastSegment.find(',', lastSegment.find(',') + 1))) << tags[0];
        delete score;
    }
}

TEST_F(Mei_Tests, mei_export_slur_uses_only_owned_endpoint_system_segments) {
    auto exportFunc = [](Score* score, const muse::io::path_t& path) -> Err {
        MeiWriter meiWriter;
        return meiWriter.writeScore(score, path);
    };

    MasterScore* score = ScoreRW::readScore(MEI_DIR + u"slur-01.mscx", false);
    ASSERT_TRUE(score);
    score->setLayoutAll();
    score->doLayout();

    Slur* slur = nullptr;
    std::vector<Spanner*> otherSlurs;
    for (const auto& entry : score->spannerMap().map()) {
        Spanner* candidate = entry.second;
        if (!candidate || !candidate->isSlur()) {
            continue;
        }
        Slur* candidateSlur = toSlur(candidate);
        const ChordRest* start = candidateSlur->startCR();
        const ChordRest* end = candidateSlur->endCR();
        const System* startSystem = start && start->measure() ? start->measure()->system() : nullptr;
        const System* endSystem = end && end->measure() ? end->measure()->system() : nullptr;
        if (!slur && startSystem && startSystem == endSystem && candidateSlur->nsegments() == 1) {
            slur = candidateSlur;
        } else {
            otherSlurs.push_back(candidate);
        }
    }
    ASSERT_TRUE(slur);
    for (Spanner* other : otherSlurs) {
        score->removeSpanner(other);
    }

    const ChordRest* endAnchor = slur->endCR();
    ASSERT_TRUE(endAnchor);
    System* endpointSystem = endAnchor->measure()->system();
    ASSERT_TRUE(endpointSystem);
    Page* endpointPage = endpointSystem->page();
    ASSERT_TRUE(endpointPage);
    const size_t legitimateSegmentCount = slur->nsegments();

    // A recycled System can retain a real Page pointer after Page::systems()
    // has stopped owning it.
    System* recycledSystem = Factory::createSystem(endpointPage);
    recycledSystem->moveToPage(endpointPage);
    ASSERT_EQ(recycledSystem->page(), endpointPage);
    ASSERT_EQ(std::find(endpointPage->systems().cbegin(), endpointPage->systems().cend(), recycledSystem),
              endpointPage->systems().cend());
    SlurSegment* recycledSegment = new SlurSegment(recycledSystem);
    recycledSegment->setSystem(recycledSystem);
    recycledSegment->setPos(PointF(500.0, 500.0));
    slur->add(recycledSegment);

    // This initialized System is genuinely page-owned, but follows both
    // musical endpoints and therefore cannot own part of this slur.
    System* outsideSystem = Factory::createSystem(endpointPage);
    outsideSystem->adjustStavesNumber(score->nstaves());
    endpointPage->appendSystem(outsideSystem);
    ASSERT_EQ(outsideSystem->page(), endpointPage);
    SlurSegment* outsideSegment = new SlurSegment(outsideSystem);
    outsideSegment->setSystem(outsideSystem);
    outsideSegment->setPos(PointF(600.0, 600.0));
    slur->add(outsideSegment);
    ASSERT_EQ(slur->nsegments(), legitimateSegmentCount + 2u);

    const String outputName = u"pianomania-slur-owned-endpoint-systems.test.mei";
    ASSERT_TRUE(ScoreRW::saveScore(score, outputName, exportFunc));
    const std::vector<std::string> tags =
        collectStartTags(readTestTextFile(outputName), "slur");
    ASSERT_EQ(tags.size(), 1u);
    const std::optional<std::string> bezier = xmlAttributeValue(tags.front(), "pm:bezier");
    const std::optional<std::string> extent = xmlAttributeValue(tags.front(), "pm:x1y1x2y2");
    ASSERT_TRUE(bezier.has_value()) << tags.front();
    ASSERT_TRUE(extent.has_value()) << tags.front();
    EXPECT_EQ(countSegments(*bezier), legitimateSegmentCount) << tags.front();

    const size_t lastSeparator = bezier->rfind(';');
    const std::string firstCurve = bezier->substr(0, bezier->find(';'));
    const std::string lastCurve =
        lastSeparator == std::string::npos ? *bezier : bezier->substr(lastSeparator + 1);
    const std::vector<double> firstCoordinates = parseCoordinates(firstCurve);
    const std::vector<double> lastCoordinates = parseCoordinates(lastCurve);
    const std::vector<double> extentCoordinates = parseCoordinates(*extent);
    ASSERT_EQ(firstCoordinates.size(), 8u);
    ASSERT_EQ(lastCoordinates.size(), 8u);
    ASSERT_EQ(extentCoordinates.size(), 4u);
    EXPECT_NEAR(extentCoordinates[0], firstCoordinates[0], 0.001);
    EXPECT_NEAR(extentCoordinates[1], firstCoordinates[1], 0.001);
    EXPECT_NEAR(extentCoordinates[2], lastCoordinates[6], 0.001);
    EXPECT_NEAR(extentCoordinates[3], lastCoordinates[7], 0.001);
    delete recycledSystem;
    delete score;
}

TEST_F(Mei_Tests, mei_export_keeps_legitimate_multi_system_slur_segments) {
    auto exportFunc = [](Score* score, const muse::io::path_t& path) -> Err {
        MeiWriter meiWriter;
        return meiWriter.writeScore(score, path);
    };

    MasterScore* score = ScoreRW::readScore(MEI_DIR + u"slur-02.mscx", false);
    ASSERT_TRUE(score);
    score->setLayoutAll();
    score->doLayout();

    Slur* slur = nullptr;
    std::vector<Spanner*> otherSlurs;
    for (const auto& entry : score->spannerMap().map()) {
        Spanner* candidate = entry.second;
        if (!candidate || !candidate->isSlur()) {
            continue;
        }
        Slur* candidateSlur = toSlur(candidate);
        const ChordRest* start = candidateSlur->startCR();
        const ChordRest* end = candidateSlur->endCR();
        const System* startSystem = start && start->measure() ? start->measure()->system() : nullptr;
        const System* endSystem = end && end->measure() ? end->measure()->system() : nullptr;
        if (!slur && startSystem && endSystem && startSystem != endSystem
            && candidateSlur->nsegments() >= 2) {
            slur = candidateSlur;
        } else {
            otherSlurs.push_back(candidate);
        }
    }
    ASSERT_TRUE(slur);
    for (Spanner* other : otherSlurs) {
        score->removeSpanner(other);
    }
    const size_t segmentCount = slur->nsegments();

    const String outputName = u"pianomania-legitimate-multi-system-slur.test.mei";
    ASSERT_TRUE(ScoreRW::saveScore(score, outputName, exportFunc));
    const std::vector<std::string> tags =
        collectStartTags(readTestTextFile(outputName), "slur");
    ASSERT_EQ(tags.size(), 1u);
    const std::optional<std::string> bezier = xmlAttributeValue(tags.front(), "pm:bezier");
    const std::optional<std::string> extent = xmlAttributeValue(tags.front(), "pm:x1y1x2y2");
    ASSERT_TRUE(bezier.has_value()) << tags.front();
    ASSERT_TRUE(extent.has_value()) << tags.front();
    EXPECT_EQ(countSegments(*bezier), segmentCount) << tags.front();

    const size_t lastSeparator = bezier->rfind(';');
    const std::string firstCurve = bezier->substr(0, bezier->find(';'));
    const std::string lastCurve =
        lastSeparator == std::string::npos ? *bezier : bezier->substr(lastSeparator + 1);
    const std::vector<double> firstCoordinates = parseCoordinates(firstCurve);
    const std::vector<double> lastCoordinates = parseCoordinates(lastCurve);
    const std::vector<double> extentCoordinates = parseCoordinates(*extent);
    ASSERT_EQ(firstCoordinates.size(), 8u);
    ASSERT_EQ(lastCoordinates.size(), 8u);
    ASSERT_EQ(extentCoordinates.size(), 4u);
    EXPECT_NEAR(extentCoordinates[0], firstCoordinates[0], 0.001);
    EXPECT_NEAR(extentCoordinates[1], firstCoordinates[1], 0.001);
    EXPECT_NEAR(extentCoordinates[2], lastCoordinates[6], 0.001);
    EXPECT_NEAR(extentCoordinates[3], lastCoordinates[7], 0.001);
    delete score;
}

TEST_F(Mei_Tests, mei_export_hairpins_ending_without_onset_bind_end_element) {
    auto exportFunc = [](Score* score, const muse::io::path_t& path) -> Err {
        MeiWriter meiWriter;
        return meiWriter.writeScore(score, path);
    };

    MasterScore* score = ScoreRW::readScore(MEI_DIR + u"hairpin-01.mscx", false);
    ASSERT_TRUE(score);

    Chord* sustainedChord = nullptr;
    ChordRest* lastChordRest = nullptr;
    for (Segment* segment = score->firstSegment(SegmentType::ChordRest); segment;
         segment = segment->next1(SegmentType::ChordRest)) {
        EngravingItem* item = segment->element(0);
        if (!item || !item->isChordRest()) {
            continue;
        }
        lastChordRest = toChordRest(item);
        if (!sustainedChord && item->isChord() && toChord(item)->actualTicks() >= Fraction(1, 4)) {
            sustainedChord = toChord(item);
        }
    }
    ASSERT_TRUE(sustainedChord);
    ASSERT_TRUE(lastChordRest);

    const auto addHairpin = [score](const Fraction& tick, const Fraction& tick2) {
        Hairpin* hairpin = Factory::createHairpin(score->dummy()->segment());
        hairpin->setHairpinType(HairpinType::DIM_HAIRPIN);
        hairpin->setTick(tick);
        hairpin->setTick2(tick2);
        hairpin->setTrack(0);
        hairpin->setTrack2(0);
        hairpin->setAnchor(Spanner::Anchor::SEGMENT);
        score->addSpanner(hairpin);
    };
    // One hairpin ends inside a sustained note; the other ends at the final barline.
    addHairpin(sustainedChord->tick(), sustainedChord->tick() + sustainedChord->actualTicks() * Fraction(1, 2));
    addHairpin(lastChordRest->tick(), score->endTick());
    score->setLayoutAll();
    score->doLayout();

    const String outputName = u"pianomania-hairpin-end-without-onset.test.mei";
    ASSERT_TRUE(ScoreRW::saveScore(score, outputName, exportFunc));
    delete score;

    const std::string meiText = readTestTextFile(outputName);
    const std::vector<std::string> hairpinTags = collectStartTags(meiText, "hairpin");
    ASSERT_GE(hairpinTags.size(), 2u);
    for (const std::string& tag : hairpinTags) {
        const std::optional<std::string> endId = xmlAttributeValue(tag, "endid");
        ASSERT_TRUE(endId.has_value()) << tag;
        EXPECT_NE(meiText.find("xml:id=\"" + endId->substr(1) + "\""), std::string::npos) << tag;
    }

    const std::vector<std::string> measureRestTags = collectStartTags(meiText, "mRest");
    ASSERT_FALSE(measureRestTags.empty());
    for (const std::string& tag : measureRestTags) {
        EXPECT_TRUE(xmlAttributeValue(tag, "pm:xy").has_value()) << tag;
    }
}

TEST_F(Mei_Tests, mei_export_hidden_tuplet_semantic_visibility_matches_drawn_geometry) {
    auto exportFunc = [](Score* score, const muse::io::path_t& path) -> Err {
        MeiWriter meiWriter;
        return meiWriter.writeScore(score, path);
    };

    MasterScore* score = ScoreRW::readScore(MEI_DIR + u"tuplet-03.mscx", false);
    ASSERT_TRUE(score);
    Tuplet* hiddenTuplet = nullptr;
    for (Segment* segment = score->firstSegment(SegmentType::ChordRest); segment && !hiddenTuplet;
         segment = segment->next1(SegmentType::ChordRest)) {
        for (EngravingItem* item : segment->elist()) {
            if (item && item->isChordRest() && toChordRest(item)->tuplet()) {
                hiddenTuplet = toChordRest(item)->tuplet();
                break;
            }
        }
    }
    ASSERT_TRUE(hiddenTuplet);
    // A tuplet hidden in the score keeps its number type, as in published catalog scores.
    hiddenTuplet->setVisible(false);
    score->setLayoutAll();
    score->doLayout();

    const String outputName = u"pianomania-hidden-tuplet-visibility.test.mei";
    ASSERT_TRUE(ScoreRW::saveScore(score, outputName, exportFunc));
    delete score;

    const std::vector<std::string> tags = collectStartTags(readTestTextFile(outputName), "tuplet");
    ASSERT_FALSE(tags.empty());
    size_t hiddenTags = 0;
    for (const std::string& tag : tags) {
        const bool drawnNumber = xmlAttributeValue(tag, "pm:tuplet-number-visible") == "true";
        const bool drawnBracket = xmlAttributeValue(tag, "pm:tuplet-bracket-visible") == "true";
        EXPECT_EQ(xmlAttributeValue(tag, "num.visible") != "false", drawnNumber) << tag;
        if (!drawnBracket) {
            EXPECT_NE(xmlAttributeValue(tag, "bracket.visible"), "true") << tag;
        }
        if (!drawnNumber && !drawnBracket) {
            ++hiddenTags;
        }
    }
    EXPECT_GE(hiddenTags, 1u);
}

TEST_F(Mei_Tests, mei_breaks_01) {
    meiReadTest("breaks-01");
}

TEST_F(Mei_Tests, mei_breath_01) {
    meiReadTest("breath-01");
}

TEST_F(Mei_Tests, mei_btrem_01) {
    meiReadTest("btrem-01");
}

TEST_F(Mei_Tests, mei_chord_label_01) {
    meiReadTest("chord-label-01");
}

TEST_F(Mei_Tests, mei_clef_01) {
    meiReadTest("clef-01");
}

TEST_F(Mei_Tests, mei_color_01) {
    meiReadTest("color-01");
}

TEST_F(Mei_Tests, mei_cross_staff_01) {
    meiReadTest("cross-staff-01");
}

TEST_F(Mei_Tests, mei_dir_01) {
    meiReadTest("dir-01");
}

TEST_F(Mei_Tests, mei_dynamic_01) {
    meiReadTest("dynamic-01");
}

TEST_F(Mei_Tests, mei_ending_01) {
    meiReadTest("ending-01");
}

TEST_F(Mei_Tests, mei_fermata_01) {
    meiReadTest("fermata-01");
}

TEST_F(Mei_Tests, mei_fig_bass_01) {
    meiReadTest("fig-bass-01");
}

TEST_F(Mei_Tests, mei_fingering_01) {
    meiReadTest("fingering-01");
}

TEST_F(Mei_Tests, mei_pianomania_fingering_notation_obstacles) {
    std::string baselineText;
    std::string prettifyText;
    std::string forceText;
    int baselineNormalizedManualFingerings = 0;
    int prettifyNormalizedManualFingerings = 0;
    int forceNormalizedManualFingerings = 0;
    const std::vector<FingeringExportData> baselineFingerings = exportPianomaniaFingeringObstacleFixture(
        false, false, u"pianomania-fingering-obstacles.baseline.test.mei", &baselineText, &baselineNormalizedManualFingerings);
    const std::vector<FingeringExportData> prettifyFingerings = exportPianomaniaFingeringObstacleFixture(
        true, false, u"pianomania-fingering-obstacles.prettify.test.mei", &prettifyText, &prettifyNormalizedManualFingerings);
    const std::vector<FingeringExportData> forceFingerings = exportPianomaniaFingeringObstacleFixture(
        true, true, u"pianomania-fingering-obstacles.force.test.mei", &forceText, &forceNormalizedManualFingerings);

    ASSERT_EQ(baselineFingerings.size(), 10u);
    ASSERT_EQ(prettifyFingerings.size(), baselineFingerings.size());
    ASSERT_EQ(forceFingerings.size(), baselineFingerings.size());

    for (const FingeringExportData& fingering : prettifyFingerings) {
        EXPECT_TRUE(fingering.pmxy.has_value());
        EXPECT_TRUE(fingering.yOffset.has_value());
    }

    bool automaticFingeringMoved = false;
    for (size_t i = 0; i + 1 < prettifyFingerings.size(); ++i) {
        ASSERT_TRUE(baselineFingerings[i].yOffset.has_value());
        ASSERT_TRUE(prettifyFingerings[i].yOffset.has_value());
        if (std::abs(*baselineFingerings[i].yOffset - *prettifyFingerings[i].yOffset) > 0.05) {
            automaticFingeringMoved = true;
            break;
        }
    }
    EXPECT_TRUE(automaticFingeringMoved);

    const FingeringExportData& baselineManual = baselineFingerings.back();
    const FingeringExportData& prettifyManual = prettifyFingerings.back();
    const FingeringExportData& forceManual = forceFingerings.back();
    ASSERT_TRUE(baselineManual.yOffset.has_value());
    ASSERT_TRUE(prettifyManual.yOffset.has_value());
    ASSERT_TRUE(forceManual.yOffset.has_value());
    EXPECT_NEAR(*baselineManual.yOffset, *prettifyManual.yOffset, 0.1);
    EXPECT_EQ(baselineNormalizedManualFingerings, 0);
    EXPECT_EQ(prettifyNormalizedManualFingerings, 0);
    EXPECT_GE(forceNormalizedManualFingerings, 1);
    EXPECT_GT(std::abs(*forceManual.yOffset - *prettifyManual.yOffset), 0.05);
    EXPECT_NE(forceManual.pmxy, prettifyManual.pmxy);
}

TEST_F(Mei_Tests, mei_ftrem_01) {
    meiReadTest("ftrem-01");
}

TEST_F(Mei_Tests, mei_glisss_01) {
    meiReadTest("gliss-01");
}

TEST_F(Mei_Tests, mei_gracenote_01) {
    meiReadTest("gracenote-01");
}

TEST_F(Mei_Tests, mei_gracenote_02) {
    meiReadTest("gracenote-02");
}

TEST_F(Mei_Tests, mei_hairpin_01) {
    meiReadTest("hairpin-01");
}

TEST_F(Mei_Tests, mei_export_wedge_keeps_only_owned_endpoint_system_geometry) {
    auto exportFunc = [](Score* score, const muse::io::path_t& path) -> Err {
        MeiWriter meiWriter;
        return meiWriter.writeScore(score, path);
    };
    MasterScore* score = ScoreRW::readScore(MEI_DIR + u"hairpin-01.mscx", false);
    ASSERT_TRUE(score);
    score->setLayoutAll();
    score->doLayout();

    Hairpin* hairpin = nullptr;
    std::vector<Spanner*> others;
    for (const auto& entry : score->spannerMap().map()) {
        Spanner* candidate = entry.second;
        if (!candidate || !candidate->isHairpin()) {
            continue;
        }
        Hairpin* wedge = toHairpin(candidate);
        if (!hairpin && !wedge->isLineType() && !wedge->segmentsEmpty()) {
            hairpin = wedge;
        } else {
            others.push_back(candidate);
        }
    }
    ASSERT_TRUE(hairpin);
    for (Spanner* other : others) {
        score->removeSpanner(other);
    }
    const size_t legitimateCount = hairpin->spannerSegments().size();
    const String baselineName = u"pianomania-wedge-owned-baseline.test.mei";
    ASSERT_TRUE(ScoreRW::saveScore(score, baselineName, exportFunc));
    const auto baselineTags = collectStartTags(readTestTextFile(baselineName), "hairpin");
    ASSERT_EQ(baselineTags.size(), 1u);

    Page* page = hairpin->frontSegment()->system()->page();
    ASSERT_TRUE(page);
    System* recycled = Factory::createSystem(page);
    recycled->moveToPage(page);
    ASSERT_EQ(recycled->page(), page);
    ASSERT_EQ(std::find(page->systems().cbegin(), page->systems().cend(), recycled),
              page->systems().cend());
    System* outside = Factory::createSystem(score->pages().back());
    outside->adjustStavesNumber(score->nstaves());
    score->pages().back()->appendSystem(outside);
    const auto* original = toHairpinSegment(hairpin->frontSegment());
    for (System* invalidSystem : {recycled, outside}) {
        auto* ghost = toHairpinSegment(hairpin->createLineSegment(invalidSystem));
        ghost->setSystem(invalidSystem);
        ghost->setPos(PointF(600.0, 600.0));
        ghost->setPos2(PointF(160.0, 0.0));
        ghost->mutldata()->points = original->ldata()->points;
        ghost->mutldata()->npoints = original->ldata()->npoints;
        hairpin->add(ghost);
    }
    ASSERT_EQ(hairpin->spannerSegments().size(), legitimateCount + 2u);
    const String outputName = u"pianomania-wedge-owned-systems.test.mei";
    ASSERT_TRUE(ScoreRW::saveScore(score, outputName, exportFunc));
    const auto tags = collectStartTags(readTestTextFile(outputName), "hairpin");
    ASSERT_EQ(tags.size(), 1u);
    for (const char* attribute : {"pm:segments", "pm:hairpin-lines", "pm:x1y1x2y2"}) {
        const auto baseline = xmlAttributeValue(baselineTags.front(), attribute);
        const auto actual = xmlAttributeValue(tags.front(), attribute);
        ASSERT_TRUE(baseline.has_value()) << attribute;
        ASSERT_TRUE(actual.has_value()) << attribute;
        EXPECT_EQ(*actual, *baseline) << attribute;
    }
    delete recycled;
    delete score;
}

TEST_F(Mei_Tests, mei_export_articulation_centres_match_native_rendered_bounds) {
    for (const String fixture : {u"artic-01.mscx"}) {
        SCOPED_TRACE(fixture.toStdString());
        MasterScore* score = ScoreRW::readScore(MEI_DIR + fixture, false);
        ASSERT_TRUE(score);
        score->setLayoutAll();
        score->doLayout();
        std::vector<std::array<double, 2>> expected;
        score->scanElements([&](EngravingItem* item) {
            if (!item->isArticulation()) {
                return;
            }
            const RectF bounds = item->pageBoundingRect();
            expected.push_back({bounds.center().x() / DPI,
                score->style().styleD(Sid::pageHeight) - bounds.center().y() / DPI});
        });
        ASSERT_FALSE(expected.empty());
        std::string output;
        ASSERT_TRUE(pmWriteMeiToString(score, true, output));
        std::vector<std::array<double, 2>> actual;
        bool distinctFromOrigin = false;
        for (const auto& tag : collectStartTags(output, "artic")) {
            const auto center = xmlAttributeValue(tag, "pm:artic-center");
            ASSERT_TRUE(center.has_value()) << tag;
            const auto coordinates = parseCoordinates(*center);
            ASSERT_EQ(coordinates.size(), 2u);
            actual.push_back({coordinates[0], coordinates[1]});
            distinctFromOrigin |= center != xmlAttributeValue(tag, "pm:xy");
        }
        ASSERT_EQ(actual.size(), expected.size());
        // Export rounding can merge neighboring X values. Match complete
        // native centres instead of relying on a rounded sort order.
        for (const auto& center : actual) {
            const auto match = std::find_if(expected.begin(), expected.end(),
                [&](const auto& reference) {
                    return std::abs(center[0] - reference[0]) <= 0.00051
                        && std::abs(center[1] - reference[1]) <= 0.00051;
                });
            ASSERT_NE(match, expected.end());
            expected.erase(match);
        }
        EXPECT_TRUE(expected.empty());
        EXPECT_TRUE(distinctFromOrigin);
        delete score;
    }
}

TEST_F(Mei_Tests, mei_export_articulation_centres_preserve_native_notehead_anchors) {
    for (double authoredOffsetSp : {0.0, 0.65}) {
        SCOPED_TRACE(authoredOffsetSp);
        MasterScore* score = ScoreRW::readScore(MEI_DIR + u"artic-01.mscx", false);
        ASSERT_TRUE(score);
        score->scanElements([&](EngravingItem* item) {
            if (item->isArticulation() && authoredOffsetSp != 0.0) {
                item->setAutoplace(false);
                item->setOffset(PointF(authoredOffsetSp * score->style().spatium(), 0.0));
            }
        });
        score->setLayoutAll();
        score->doLayout();
        std::vector<std::array<double, 4>> expected;
        bool coversSingleNote = false;
        bool coversChord = false;
        score->scanElements([&](EngravingItem* item) {
            if (!item->isArticulation()) {
                return;
            }
            const Articulation* articulation = toArticulation(item);
            const ChordRest* cr = articulation->chordRest();
            if (!cr || !cr->isChord()) {
                return;
            }
            const Chord* chord = toChord(cr);
            coversSingleNote |= chord->notes().size() == 1u;
            coversChord |= chord->notes().size() > 1u;
            RectF heads;
            bool first = true;
            for (const Note* note : chord->notes()) {
                if (!note->visible()) {
                    continue;
                }
                const RectF rendered = note->pageBoundingRect();
                heads = first ? rendered : heads.united(rendered);
                first = false;
            }
            const RectF bounds = articulation->pageBoundingRect();
            expected.push_back({bounds.center().x() / DPI,
                score->style().styleD(Sid::pageHeight) - bounds.center().y() / DPI,
                heads.center().x() / DPI,
                score->style().styleD(Sid::pageHeight) - heads.center().y() / DPI});
        });
        ASSERT_TRUE(coversSingleNote);
        ASSERT_TRUE(coversChord);
        ASSERT_FALSE(expected.empty());
        std::string output;
        ASSERT_TRUE(pmWriteMeiToString(score, true, output));
        const auto tags = collectStartTags(output, "artic");
        ASSERT_EQ(tags.size(), expected.size());
        bool preservesAuthoredOffset = false;
        for (const auto& tag : tags) {
            const auto center = xmlAttributeValue(tag, "pm:artic-center");
            const auto anchor = xmlAttributeValue(tag, "pm:artic-anchor-center");
            ASSERT_TRUE(center.has_value());
            ASSERT_TRUE(anchor.has_value()) << tag;
            const auto c = parseCoordinates(*center);
            const auto a = parseCoordinates(*anchor);
            ASSERT_EQ(c.size(), 2u);
            ASSERT_EQ(a.size(), 2u);
            const auto match = std::find_if(expected.begin(), expected.end(),
                [&](const auto& reference) {
                    return std::abs(c[0] - reference[0]) <= 0.00051
                        && std::abs(c[1] - reference[1]) <= 0.00051
                        && std::abs(a[0] - reference[2]) <= 0.0000051
                        && std::abs(a[1] - reference[3]) <= 0.0000051;
                });
            ASSERT_NE(match, expected.end()) << tag;
            preservesAuthoredOffset |= std::abs(c[0] - a[0]) > 0.5 * score->style().spatium() / DPI;
            expected.erase(match);
        }
        EXPECT_TRUE(expected.empty());
        if (authoredOffsetSp != 0.0) {
            EXPECT_TRUE(preservesAuthoredOffset);
        }
        delete score;
    }
}


TEST_F(Mei_Tests, mei_hairpin_export_includes_pm_hairpin_lines_when_endpoints_present) {
    auto exportFunc = [](Score* score, const muse::io::path_t& path) -> Err {
        MeiWriter meiWriter;
        return meiWriter.writeScore(score, path);
    };

    MasterScore* score = ScoreRW::readScore(MEI_DIR + u"hairpin-01.mscx", false);
    ASSERT_TRUE(score);

    score->style().set(Sid::dynamicsHairpinsAutoCenterOnGrandStaff, true);
    score->setLayoutAll();
    score->doLayout();

    const String outputName = u"hairpin-01.pm-lines.test.mei";
    bool output = ScoreRW::saveScore(score, outputName, exportFunc);
    EXPECT_TRUE(output);
    delete score;

    muse::io::File outputFile(outputName);
    ASSERT_TRUE(outputFile.open(muse::io::IODevice::ReadOnly));
    auto outputData = outputFile.readAll();
    std::string meiText(reinterpret_cast<const char*>(outputData.constData()), outputData.size());

    size_t hairpinTagCount = 0;
    size_t inspectedTagCount = 0;
    size_t crossPageHairpinCount = 0;
    size_t cursor = 0;

    while ((cursor = meiText.find("<hairpin", cursor)) != std::string::npos) {
        size_t end = meiText.find('>', cursor);
        ASSERT_NE(end, std::string::npos);

        std::string tag = meiText.substr(cursor, end - cursor + 1);
        hairpinTagCount++;

        if (tag.find("pm:x1y1x2y2=") != std::string::npos) {
            inspectedTagCount++;
            EXPECT_NE(tag.find("pm:hairpin-lines="), std::string::npos);

            const std::optional<std::string> lines = xmlAttributeValue(tag, "pm:hairpin-lines");
            ASSERT_TRUE(lines.has_value());
            std::vector<int> pageIndexes;
            size_t entryStart = 0;
            while (entryStart < lines->size()) {
                size_t entryEnd = lines->find(';', entryStart);
                const std::string entry = lines->substr(entryStart, entryEnd - entryStart);
                int pageIndex = -1;
                ASSERT_EQ(std::sscanf(entry.c_str(), "%d,", &pageIndex), 1);
                ASSERT_GE(pageIndex, 0);
                if (!pageIndexes.empty()) {
                    EXPECT_GE(pageIndex, pageIndexes.back());
                }
                pageIndexes.push_back(pageIndex);
                if (entryEnd == std::string::npos) {
                    break;
                }
                entryStart = entryEnd + 1;
            }

            ASSERT_FALSE(pageIndexes.empty());
            if (pageIndexes.front() != pageIndexes.back()) {
                crossPageHairpinCount++;
            }
        }

        cursor = end + 1;
    }

    EXPECT_GT(hairpinTagCount, 0u);
    EXPECT_GE(hairpinTagCount, inspectedTagCount);
    EXPECT_GT(crossPageHairpinCount, 0u);

    size_t centeredHairpinCount = 0;
    size_t nonCenteredHairpinCount = 0;
    for (const std::string& tag : collectStartTags(meiText, "hairpin")) {
        const std::optional<std::string> centered = xmlAttributeValue(tag, "centerBetweenStaves");
        ASSERT_TRUE(centered.has_value()) << tag;
        if (centered == "true") {
            centeredHairpinCount++;
        } else if (centered == "false") {
            nonCenteredHairpinCount++;
        }
    }
    EXPECT_GT(centeredHairpinCount, 0u);
    EXPECT_GT(nonCenteredHairpinCount, 0u);
}

TEST_F(Mei_Tests, mei_line_hairpin_exports_resolved_center_and_layout_y_offset) {
    auto exportFunc = [](Score* score, const muse::io::path_t& path) -> Err {
        MeiWriter meiWriter;
        return meiWriter.writeScore(score, path);
    };

    MasterScore* score = ScoreRW::readScore(MEI_DIR + u"hairpin-01.mscx", false);
    ASSERT_TRUE(score);

    Hairpin* lineHairpin = nullptr;
    for (const auto& pair : score->spanner()) {
        Spanner* spanner = pair.second;
        if (!spanner || !spanner->isHairpin()) {
            continue;
        }

        Hairpin* hairpin = toHairpin(spanner);
        if (hairpin->staffIdx() == 0 && !hairpin->isLineType()) {
            lineHairpin = hairpin;
            break;
        }
    }
    ASSERT_TRUE(lineHairpin);

    lineHairpin->setHairpinType(HairpinType::CRESC_LINE);
    lineHairpin->setCenterBetweenStaves(AutoOnOff::AUTO);
    score->style().set(Sid::dynamicsHairpinsAutoCenterOnGrandStaff, true);
    score->setLayoutAll();
    score->doLayout();

    ASSERT_FALSE(lineHairpin->segmentsEmpty());
    const SpannerSegment* firstSegment = lineHairpin->frontSegment();
    ASSERT_TRUE(firstSegment);
    const System* system = firstSegment->system();
    ASSERT_TRUE(system);
    const Staff* staff = lineHairpin->staff();
    ASSERT_TRUE(staff);

    const double spatium = staff->spatium(lineHairpin->tick());
    ASSERT_GT(spatium, 0.0);
    double staffY = system->staffYpage(staff->idx());
    if (lineHairpin->placement() == PlacementV::BELOW) {
        staffY += system->staff(staff->idx())->bbox().height();
    }
    staffY += lineHairpin->staffOffsetY();
    const double expectedYOffset = (staffY - firstSegment->pagePos().y()) / spatium;
    EXPECT_LT(expectedYOffset, 0.0);

    const String outputName = u"hairpin-01.line-center.test.mei";
    ASSERT_TRUE(ScoreRW::saveScore(score, outputName, exportFunc));
    delete score;

    const std::string meiText = readTestTextFile(outputName);
    const std::vector<std::string> directiveTags = collectStartTags(meiText, "dir");

    size_t matchingTagCount = 0;
    for (const std::string& tag : directiveTags) {
        const std::optional<std::string> type = xmlAttributeValue(tag, "type");
        if (!type.has_value() || type->find("mscore-hairpin") == std::string::npos) {
            continue;
        }

        const std::optional<double> yOffset = xmlAttributeDouble(tag, "yOffset");
        if (xmlAttributeValue(tag, "centerBetweenStaves") != "true"
            || !yOffset.has_value()
            || std::abs(*yOffset - expectedYOffset) > 0.051) {
            continue;
        }

        matchingTagCount++;
        EXPECT_LT(*yOffset, 0.0);
        EXPECT_TRUE(xmlAttributeValue(tag, "pm:xy").has_value());
        EXPECT_TRUE(xmlAttributeValue(tag, "pm:x1y1x2y2").has_value());
        EXPECT_TRUE(xmlAttributeValue(tag, "pm:segments").has_value());
    }

    EXPECT_EQ(matchingTagCount, 1u);
}

TEST_F(Mei_Tests, mei_directives_export_resolved_center_between_staves) {
    auto exportFunc = [](Score* score, const muse::io::path_t& path) -> Err {
        MeiWriter meiWriter;
        return meiWriter.writeScore(score, path);
    };

    MasterScore* score = ScoreRW::readScore(MEI_DIR + u"hairpin-01.mscx", false);
    ASSERT_TRUE(score);

    Segment* firstSegment = score->firstSegment(SegmentType::ChordRest);
    ASSERT_TRUE(firstSegment);

    Expression* expression = Factory::createExpression(firstSegment, true);
    expression->setTrack(0);
    expression->setXmlText(u"centered expression");
    expression->setCenterBetweenStaves(AutoOnOff::ON);
    expression->setVoiceAssignment(VoiceAssignment::ALL_VOICE_IN_INSTRUMENT);
    firstSegment->add(expression);

    Dynamic* customDynamic = Factory::createDynamic(firstSegment, true);
    customDynamic->setTrack(0);
    customDynamic->setDynamicType(DynamicType::OTHER);
    customDynamic->setXmlText(u"centered custom dynamic");
    customDynamic->setCenterBetweenStaves(AutoOnOff::ON);
    customDynamic->setVoiceAssignment(VoiceAssignment::ALL_VOICE_IN_INSTRUMENT);
    firstSegment->add(customDynamic);

    StaffText* staffText = Factory::createStaffText(firstSegment, TextStyleType::STAFF, true);
    staffText->setTrack(0);
    staffText->setXmlText(u"staff-relative directive");
    staffText->setPlacement(PlacementV::BELOW);
    staffText->setOffset(0.0, staffText->spatium());
    firstSegment->add(staffText);

    std::vector<Dynamic*> standardDynamics;
    for (Segment* segment = firstSegment; segment; segment = segment->next1()) {
        for (EngravingItem* annotation : segment->annotations()) {
            if (annotation && annotation->isDynamic()) {
                Dynamic* dynamic = toDynamic(annotation);
                if (dynamic->dynamicType() != DynamicType::OTHER) {
                    standardDynamics.push_back(dynamic);
                }
            }
        }
    }
    ASSERT_GE(standardDynamics.size(), 2u);
    standardDynamics[0]->setCenterBetweenStaves(AutoOnOff::ON);
    standardDynamics[1]->setCenterBetweenStaves(AutoOnOff::OFF);

    score->style().set(Sid::dynamicsHairpinsAutoCenterOnGrandStaff, true);
    score->setLayoutAll();
    score->doLayout();

    const Staff* directiveStaff = staffText->staff();
    ASSERT_TRUE(directiveStaff);
    const StaffType* directiveStaffType = directiveStaff->staffTypeForElement(staffText);
    ASSERT_TRUE(directiveStaffType);
    const double directiveLineDistance = directiveStaff->spatium(staffText->tick())
        * directiveStaffType->lineDistance().val();
    ASSERT_GT(directiveLineDistance, 0.0);
    const double expectedStaffTextYOffset =
        (((directiveStaffType->lines() - 1) * directiveLineDistance) - staffText->y())
        / directiveLineDistance;

    const String outputName = u"hairpin-01.directive-center.test.mei";
    ASSERT_TRUE(ScoreRW::saveScore(score, outputName, exportFunc));
    delete score;

    const std::string meiText = readTestTextFile(outputName);
    const std::vector<std::string> directiveTags = collectStartTags(meiText, "dir");
    size_t centeredOrdinaryDirectives = 0;
    size_t staffRelativeDirectives = 0;
    for (const std::string& tag : directiveTags) {
        const std::optional<std::string> type = xmlAttributeValue(tag, "type");
        if (type == "mscore-staff-text") {
            EXPECT_FALSE(xmlAttributeValue(tag, "centerBetweenStaves").has_value()) << tag;
            const std::optional<double> yOffset = xmlAttributeDouble(tag, "yOffset");
            ASSERT_TRUE(yOffset.has_value()) << tag;
            EXPECT_NEAR(*yOffset, expectedStaffTextYOffset, 0.051);
            staffRelativeDirectives++;
            continue;
        }
        if (type.has_value() && type != "mscore-") {
            continue;
        }
        EXPECT_EQ(xmlAttributeValue(tag, "centerBetweenStaves"), "true") << tag;
        centeredOrdinaryDirectives++;
    }
    EXPECT_EQ(centeredOrdinaryDirectives, 2u);
    EXPECT_EQ(staffRelativeDirectives, 1u);

    size_t centeredDynamics = 0;
    size_t nonCenteredDynamics = 0;
    for (const std::string& tag : collectStartTags(meiText, "dynam")) {
        const std::optional<std::string> centered = xmlAttributeValue(tag, "centerBetweenStaves");
        ASSERT_TRUE(centered.has_value()) << tag;
        if (centered == "true") {
            centeredDynamics++;
        } else if (centered == "false") {
            nonCenteredDynamics++;
        }
    }
    EXPECT_GE(centeredDynamics, 1u);
    EXPECT_GE(nonCenteredDynamics, 1u);
}

TEST_F(Mei_Tests, mei_export_omits_invisible_note_and_idx) {
    auto exportFunc = [](Score* score, const muse::io::path_t& path) -> Err {
        MeiWriter meiWriter;
        return meiWriter.writeScore(score, path);
    };

    MasterScore* score = ScoreRW::readScore(MEI_DIR + u"beam-01.mscx", false);
    ASSERT_TRUE(score);

    const String baselineOutputName = u"beam-01.visibility-baseline.test.mei";
    const String modifiedOutputName = u"beam-01.visibility-modified.test.mei";

    bool baselineOutput = ScoreRW::saveScore(score, baselineOutputName, exportFunc);
    ASSERT_TRUE(baselineOutput);

    Note* firstExportableNote = nullptr;
    for (Segment* segment = score->firstSegment(SegmentType::ChordRest); segment && !firstExportableNote; segment = segment->next1()) {
        for (track_idx_t track = 0; track < score->ntracks() && !firstExportableNote; ++track) {
            EngravingItem* item = segment->element(track);
            if (!item || !item->isChord()) {
                continue;
            }

            Chord* chord = toChord(item);
            for (Note* note : chord->notes()) {
                if (!note->visible()) {
                    continue;
                }

                firstExportableNote = note;
                break;
            }
        }
    }
    ASSERT_TRUE(firstExportableNote);

    firstExportableNote->setVisible(false);

    bool modifiedOutput = ScoreRW::saveScore(score, modifiedOutputName, exportFunc);
    ASSERT_TRUE(modifiedOutput);
    delete score;

    auto readFile = [](const String& fileName) {
        muse::io::File file(fileName);
        EXPECT_TRUE(file.open(muse::io::IODevice::ReadOnly));
        if (!file.isOpen()) {
            return std::string();
        }

        auto data = file.readAll();
        return std::string(reinterpret_cast<const char*>(data.constData()), data.size());
    };

    const std::string baselineText = readFile(baselineOutputName);
    const std::string modifiedText = readFile(modifiedOutputName);

    auto countOccurrences = [](const std::string& text, const std::string& needle) {
        size_t count = 0;
        size_t cursor = 0;
        while ((cursor = text.find(needle, cursor)) != std::string::npos) {
            ++count;
            cursor += needle.size();
        }
        return count;
    };

    const size_t baselineNoteCount = countOccurrences(baselineText, "<note");
    const size_t modifiedNoteCount = countOccurrences(modifiedText, "<note");
    const size_t baselineIdxCount = countOccurrences(baselineText, "idx=\"");
    const size_t modifiedIdxCount = countOccurrences(modifiedText, "idx=\"");

    ASSERT_GT(baselineNoteCount, 0u);
    ASSERT_GT(baselineIdxCount, 0u);
    EXPECT_EQ(modifiedNoteCount, baselineNoteCount - 1);
    EXPECT_EQ(modifiedIdxCount, baselineIdxCount - 1);

    MasterScore* spannerScore = ScoreRW::readScore(MEI_DIR + u"color-01.mscx", false);
    ASSERT_TRUE(spannerScore);

    size_t mixedVisibilityChordCount = 0;
    for (Segment* segment = spannerScore->firstSegment(SegmentType::ChordRest); segment; segment = segment->next1()) {
        for (track_idx_t track = 0; track < spannerScore->ntracks(); ++track) {
            EngravingItem* item = segment->element(track);
            if (!item || !item->isChord()) {
                continue;
            }

            Chord* chord = toChord(item);
            if (chord->notes().empty()) {
                continue;
            }

            Note* sourceNote = chord->notes().front();
            Note* invisibleNote = Factory::createNote(chord);
            invisibleNote->setPitch(sourceNote->pitch(), sourceNote->tpc1(), sourceNote->tpc2());
            invisibleNote->setVisible(false);
            chord->add(invisibleNote);
            ++mixedVisibilityChordCount;
        }
    }
    ASSERT_GT(mixedVisibilityChordCount, 0u);

    const String spannerOutputName = u"color-01.visibility-spanners.test.mei";
    bool spannerOutput = ScoreRW::saveScore(spannerScore, spannerOutputName, exportFunc);
    ASSERT_TRUE(spannerOutput);
    delete spannerScore;

    const std::string spannerText = readFile(spannerOutputName);
    std::set<std::string> declaredIds;
    size_t declarationCursor = 0;
    const std::string declarationPrefix = "xml:id=\"";
    while ((declarationCursor = spannerText.find(declarationPrefix, declarationCursor)) != std::string::npos) {
        size_t valueStart = declarationCursor + declarationPrefix.size();
        size_t valueEnd = spannerText.find('"', valueStart);
        ASSERT_NE(valueEnd, std::string::npos);
        declaredIds.insert(spannerText.substr(valueStart, valueEnd - valueStart));
        declarationCursor = valueEnd + 1;
    }

    const std::vector<std::string> identityAttributes {
        "pm:covered-id", "pm:covered-ids", "pm:coveredUuids",
        "pm:post-terminal-ids", "pm:terminal-ids"
    };
    std::set<std::string> observedAttributes;
    for (const std::string& attributeName : identityAttributes) {
        const std::string attributePrefix = attributeName + "=\"";
        size_t attributeCursor = 0;
        while ((attributeCursor = spannerText.find(attributePrefix, attributeCursor)) != std::string::npos) {
            observedAttributes.insert(attributeName);
            size_t valueStart = attributeCursor + attributePrefix.size();
            size_t valueEnd = spannerText.find('"', valueStart);
            ASSERT_NE(valueEnd, std::string::npos);

            std::istringstream tokens(spannerText.substr(valueStart, valueEnd - valueStart));
            std::string token;
            while (tokens >> token) {
                EXPECT_NE(declaredIds.find(token), declaredIds.end())
                    << attributeName << " references undeclared " << token;
            }
            attributeCursor = valueEnd + 1;
        }
    }

    EXPECT_NE(observedAttributes.find("pm:coveredUuids"), observedAttributes.end());
    EXPECT_NE(observedAttributes.find("pm:covered-id"), observedAttributes.end());
    EXPECT_TRUE(observedAttributes.find("pm:covered-ids") != observedAttributes.end()
                || observedAttributes.find("pm:terminal-ids") != observedAttributes.end()
                || observedAttributes.find("pm:post-terminal-ids") != observedAttributes.end());
}

TEST_F(Mei_Tests, mei_export_rehearsal_mark_on_hidden_rest) {
    auto exportFunc = [](Score* score, const muse::io::path_t& path) -> Err {
        MeiWriter meiWriter;
        return meiWriter.writeScore(score, path);
    };

    MasterScore* score = ScoreRW::readScore(MEI_DIR + u"rehearsal-hidden-rest-01.mscx", false);
    ASSERT_TRUE(score);

    const String outputName = u"rehearsal-hidden-rest-01.test.mei";
    bool output = ScoreRW::saveScore(score, outputName, exportFunc);
    ASSERT_TRUE(output);
    delete score;

    muse::io::File outputFile(outputName);
    ASSERT_TRUE(outputFile.open(muse::io::IODevice::ReadOnly));
    auto outputData = outputFile.readAll();
    std::string meiText(reinterpret_cast<const char*>(outputData.constData()), outputData.size());

    size_t rehStart = meiText.find("<reh");
    ASSERT_NE(rehStart, std::string::npos);
    size_t rehEnd = meiText.find("</reh>", rehStart);
    ASSERT_NE(rehEnd, std::string::npos);

    std::string rehElement = meiText.substr(rehStart, rehEnd - rehStart + 6);
    EXPECT_NE(rehElement.find("startid=\"#"), std::string::npos);
    EXPECT_NE(rehElement.find(">HiddenRestMark</reh>"), std::string::npos);
}

TEST_F(Mei_Tests, mei_export_spanners_on_hidden_rests_use_visible_same_staff_notes) {
    auto exportFunc = [](Score* score, const muse::io::path_t& path) -> Err {
        MeiWriter meiWriter;
        return meiWriter.writeScore(score, path);
    };

    MasterScore* score = ScoreRW::readScore(MEI_DIR + u"pianomania-spanners-on-hidden-rests.mscx", false);
    ASSERT_TRUE(score);

    score->setLayoutAll();
    score->doLayout();

    const String outputName = u"pianomania-spanners-on-hidden-rests.test.mei";
    ASSERT_TRUE(ScoreRW::saveScore(score, outputName, exportFunc));
    delete score;

    const std::string meiText = readTestTextFile(outputName);
    const std::vector<std::string> noteTags = collectStartTags(meiText, "note");
    const std::vector<std::string> hairpinTags = collectStartTags(meiText, "hairpin");
    const std::vector<std::string> octaveTags = collectStartTags(meiText, "octave");
    ASSERT_EQ(noteTags.size(), 4u);
    ASSERT_EQ(hairpinTags.size(), 1u);
    ASSERT_EQ(octaveTags.size(), 1u);

    const auto noteIdForPitch = [&](const std::string& pname, const std::string& oct) {
        std::optional<std::string> matchingId;
        for (const std::string& noteTag : noteTags) {
            if (xmlAttributeValue(noteTag, "pname") != pname
                || xmlAttributeValue(noteTag, "oct") != oct) {
                continue;
            }
            EXPECT_FALSE(matchingId.has_value()) << noteTag;
            matchingId = xmlAttributeValue(noteTag, "xml:id");
        }
        return matchingId;
    };

    const std::optional<std::string> firstNoteId = noteIdForPitch("c", "5");
    const std::optional<std::string> secondNoteId = noteIdForPitch("d", "5");
    const std::optional<std::string> ottavaEndNoteId = noteIdForPitch("c", "4");
    ASSERT_TRUE(firstNoteId.has_value()) << noteTags[0];
    ASSERT_TRUE(secondNoteId.has_value()) << noteTags[1];
    ASSERT_TRUE(ottavaEndNoteId.has_value()) << noteTags[2];
    ASSERT_NE(*firstNoteId, *secondNoteId);

    const auto expectAnchors = [&](const std::string& spannerTag, const std::string& expectedEndId) {
        const std::optional<std::string> startId = xmlAttributeValue(spannerTag, "startid");
        const std::optional<std::string> endId = xmlAttributeValue(spannerTag, "endid");
        ASSERT_TRUE(startId.has_value()) << spannerTag;
        ASSERT_TRUE(endId.has_value()) << spannerTag;
        ASSERT_GT(startId->size(), 1u);
        ASSERT_GT(endId->size(), 1u);
        EXPECT_EQ(startId->substr(1), *firstNoteId) << spannerTag;
        EXPECT_EQ(endId->substr(1), expectedEndId) << spannerTag;
    };

    expectAnchors(hairpinTags.front(), *secondNoteId);
    expectAnchors(octaveTags.front(), *ottavaEndNoteId);
}

TEST_F(Mei_Tests, mei_pianomania_grace_same_pitch_indices) {
    auto exportFunc = [](Score* score, const muse::io::path_t& path) -> Err {
        MeiWriter meiWriter;
        return meiWriter.writeScore(score, path);
    };

    MasterScore* score = ScoreRW::readScore(MEI_DIR + u"pianomania-grace-same-pitch-index.mscx", false);
    ASSERT_TRUE(score);

    const String outputName = u"pianomania-grace-same-pitch-index.test.mei";
    bool output = ScoreRW::saveScore(score, outputName, exportFunc);
    ASSERT_TRUE(output);
    delete score;

    muse::io::File file(outputName);
    ASSERT_TRUE(file.open(muse::io::IODevice::ReadOnly));
    auto data = file.readAll();
    std::string meiText(reinterpret_cast<const char*>(data.constData()), data.size());

    std::vector<std::string> gSharpTags;
    size_t cursor = 0;
    while ((cursor = meiText.find("<note", cursor)) != std::string::npos) {
        size_t end = meiText.find('>', cursor);
        ASSERT_NE(end, std::string::npos);

        std::string tag = meiText.substr(cursor, end - cursor + 1);
        if (tag.find("pname=\"g\"") != std::string::npos && tag.find("oct=\"4\"") != std::string::npos) {
            gSharpTags.push_back(tag);
        }
        cursor = end + 1;
    }

    ASSERT_EQ(gSharpTags.size(), 3u);
    EXPECT_NE(gSharpTags[0].find("idx=\"0\""), std::string::npos);
    EXPECT_NE(gSharpTags[1].find("idx=\"1\""), std::string::npos);
    EXPECT_NE(gSharpTags[2].find("idx=\"2\""), std::string::npos);
}

TEST_F(Mei_Tests, mei_harp_01) {
    meiReadTest("harp-01");
}

TEST_F(Mei_Tests, mei_jump_01) {
    meiReadTest("jump-01");
}

TEST_F(Mei_Tests, mei_jump_02) {
    meiReadTest("jump-02");
}

TEST_F(Mei_Tests, mei_key_signature_01) {
    meiReadTest("key-signature-01");
}

TEST_F(Mei_Tests, mei_midi_01) {
    meiReadTest("midi-01");
}

TEST_F(Mei_Tests, mei_label_01) {
    meiReadTest("label-01");
}

TEST_F(Mei_Tests, laissez_vibrer_01) {
    meiReadTest("laissez-vibrer-01");
}

TEST_F(Mei_Tests, mei_export_laissez_vibrer_keeps_pm_geometry) {
    auto importFunc = [](MasterScore* score, const muse::io::path_t& path) -> Err {
        MeiReader meiReader(nullptr);
        return meiReader.import(score, path);
    };
    auto exportFunc = [](Score* score, const muse::io::path_t& path) -> Err {
        MeiWriter meiWriter;
        return meiWriter.writeScore(score, path);
    };

    MasterScore* score = ScoreRW::readScore(MEI_DIR + u"laissez-vibrer-01.mei", false, importFunc);
    ASSERT_TRUE(score);

    ASSERT_TRUE(ScoreRW::saveScore(score, u"laissez-vibrer-01.setup.mscx"));

    const String outputName = u"laissez-vibrer-01.pm-geometry.test.mei";
    bool output = ScoreRW::saveScore(score, outputName, exportFunc);
    ASSERT_TRUE(output);
    delete score;

    const std::string meiText = readTestTextFile(outputName);
    const std::vector<std::string> laissezVibrerTags = collectStartTags(meiText, "lv");

    ASSERT_EQ(laissezVibrerTags.size(), 6u);
    for (const std::string& tag : laissezVibrerTags) {
        EXPECT_TRUE(xmlAttributeValue(tag, "startid").has_value());
        EXPECT_TRUE(xmlAttributeValue(tag, "pm:x1y1x2y2").has_value());
        EXPECT_TRUE(xmlAttributeValue(tag, "pm:bezier").has_value());
    }
}

TEST_F(Mei_Tests, mei_lyric_01) {
    meiReadTest("lyric-01");
}

TEST_F(Mei_Tests, mei_lyric_02) {
    meiReadTest("lyric-02");
}

TEST_F(Mei_Tests, mei_lyric_03) {
    meiReadTest("lyric-03");
}

TEST_F(Mei_Tests, mei_lyric_04) {
    meiReadTest("lyric-04");
}

TEST_F(Mei_Tests, mei_measure_01) {
    meiReadTest("measure-01");
}

TEST_F(Mei_Tests, mei_measure_02) {
    meiReadTest("measure-02");
}

TEST_F(Mei_Tests, mei_mrpt_01) {
    meiReadTest("measure-repeat-01");
}

TEST_F(Mei_Tests, mei_metadata_01) {
    meiReadTest("metadata-01");
}

TEST_F(Mei_Tests, mei_mordent_01) {
    meiReadTest("mordent-01");
}

TEST_F(Mei_Tests, mei_octave_01) {
    meiReadTest("octave-01");
}

TEST_F(Mei_Tests, mei_ornam_01) {
    meiReadTest("ornam-01");
}

TEST_F(Mei_Tests, mei_page_head_01) {
    meiReadTest("page-head-01");
}

TEST_F(Mei_Tests, mei_page_head_02) {
    meiReadTest("page-head-02");
}

TEST_F(Mei_Tests, mei_pedal_01) {
    meiReadTest("pedal-01");
}

TEST_F(Mei_Tests, mei_export_final_barline_pedal_keeps_rendered_geometry_without_endid) {
    // The last pedal runs to the final barline, so it has no end ChordRest.
    // It must not invent an endid, but its page-owned rendered segments are
    // real and must reach Practice.
    auto exportFunc = [](Score* score, const muse::io::path_t& path) -> Err {
        MeiWriter meiWriter;
        return meiWriter.writeScore(score, path);
    };

    MasterScore* score = ScoreRW::readScore(MEI_DIR + u"pedal-unresolved-end-01.mscx", false);
    ASSERT_TRUE(score);

    const String outputName = u"pedal-unresolved-end-01.test.mei";
    bool output = ScoreRW::saveScore(score, outputName, exportFunc);
    ASSERT_TRUE(output);
    delete score;

    const std::string meiText = readTestTextFile(outputName);
    const std::vector<std::string> pedalTags = collectStartTags(meiText, "pedal");

    ASSERT_GT(pedalTags.size(), 0u);

    size_t openEndedTagCount = 0;
    for (const std::string& tag : pedalTags) {
        EXPECT_TRUE(xmlAttributeValue(tag, "startid").has_value());
        EXPECT_TRUE(xmlAttributeValue(tag, "pm:x1y1x2y2").has_value());
        EXPECT_TRUE(xmlAttributeValue(tag, "pm:pedal-lines").has_value());
        if (!xmlAttributeValue(tag, "endid").has_value()) {
            openEndedTagCount++;
        }
    }

    EXPECT_GT(openEndedTagCount, 0u);
}

TEST_F(Mei_Tests, mei_export_connected_pedal_includes_owned_rendered_segments) {
    auto exportFunc = [](Score* score, const muse::io::path_t& path) -> Err {
        MeiWriter meiWriter;
        return meiWriter.writeScore(score, path);
    };

    MasterScore* score = ScoreRW::readScore(MEI_DIR + u"pedal-01.mscx", false);
    ASSERT_TRUE(score);

    std::vector<std::array<double, 5>> expectedSegments;
    const std::vector<Page*>& pages = score->pages();
    const double pageHeight = score->style().styleD(Sid::pageHeight);
    for (const auto& entry : score->spannerMap().map()) {
        const Spanner* spanner = entry.second;
        if (!spanner || !spanner->isPedal()) {
            continue;
        }
        for (const SpannerSegment* segment : spanner->spannerSegments()) {
            const System* system = segment ? segment->system() : nullptr;
            const Page* page = system ? system->page() : nullptr;
            auto pageIt = std::find(pages.cbegin(), pages.cend(), page);
            if (!segment || !page || pageIt == pages.cend()) {
                continue;
            }
            const std::vector<System*>& systems = page->systems();
            ASSERT_NE(std::find(systems.cbegin(), systems.cend(), system), systems.cend());

            const PointF start = segment->pagePos();
            const PointF end = segment->pagePos() + segment->pos2();
            expectedSegments.push_back({
                static_cast<double>(std::distance(pages.cbegin(), pageIt)),
                start.x() / DPI,
                pageHeight - (start.y() / DPI),
                end.x() / DPI,
                pageHeight - (end.y() / DPI),
            });
        }
    }
    ASSERT_FALSE(expectedSegments.empty());

    const String outputName = u"pedal-01.pm-geometry.test.mei";
    ASSERT_TRUE(ScoreRW::saveScore(score, outputName, exportFunc));
    delete score;

    const std::vector<std::string> pedalTags
        = collectStartTags(readTestTextFile(outputName), "pedal");
    std::vector<std::array<double, 5>> actualSegments;
    for (const std::string& tag : pedalTags) {
        const std::optional<std::string> lines = xmlAttributeValue(tag, "pm:pedal-lines");
        if (!lines.has_value()) {
            continue;
        }
        size_t entryStart = 0;
        while (entryStart < lines->size()) {
            const size_t entryEnd = lines->find(';', entryStart);
            const std::string entryText = lines->substr(entryStart, entryEnd - entryStart);
            std::array<double, 5> actual {};
            ASSERT_EQ(
                std::sscanf(
                    entryText.c_str(),
                    "%lf,%lf,%lf,%lf,%lf",
                    &actual[0],
                    &actual[1],
                    &actual[2],
                    &actual[3],
                    &actual[4]
                ),
                5
            );
            actualSegments.push_back(actual);
            if (entryEnd == std::string::npos) {
                break;
            }
            entryStart = entryEnd + 1;
        }
    }

    ASSERT_EQ(actualSegments.size(), expectedSegments.size());
    std::vector<bool> matched(actualSegments.size(), false);
    for (const std::array<double, 5>& expected : expectedSegments) {
        bool found = false;
        for (size_t i = 0; i < actualSegments.size(); ++i) {
            if (matched[i]) {
                continue;
            }
            bool equal = static_cast<int>(actualSegments[i][0]) == static_cast<int>(expected[0]);
            for (size_t coordinate = 1; coordinate < expected.size(); ++coordinate) {
                equal = equal && std::abs(actualSegments[i][coordinate] - expected[coordinate]) <= 0.0006;
            }
            if (equal) {
                matched[i] = true;
                found = true;
                break;
            }
        }
        EXPECT_TRUE(found);
    }
}

TEST_F(Mei_Tests, pianomania_text_directive_exports_text_box_centre) {
    // Practice centres a plain text directive's rendered text on pm:xy
    // (directives with an extender use their line start instead), so the
    // export must write the centre of the text's box.
    MasterScore* score = ScoreRW::readScore(MEI_DIR + u"pianomania-grace-same-pitch-index.mscx", false);
    ASSERT_TRUE(score);

    std::string meiText;
    ASSERT_TRUE(pmWriteMeiToString(score, true, meiText));

    const TextBase* dolce = nullptr;
    for (Segment* segment = score->firstMeasure()->first(); segment && !dolce; segment = segment->next()) {
        for (EngravingItem* item : segment->annotations()) {
            if (item->isTextBase() && toTextBase(item)->plainText() == u"dolce") {
                dolce = toTextBase(item);
            }
        }
    }
    ASSERT_TRUE(dolce);

    const size_t textPos = meiText.find(">dolce</dir>");
    ASSERT_NE(textPos, std::string::npos);
    const size_t tagPos = meiText.rfind("<dir ", textPos);
    ASSERT_NE(tagPos, std::string::npos);
    const std::optional<std::string> xy = xmlAttributeValue(meiText.substr(tagPos, textPos + 1 - tagPos), "pm:xy");
    ASSERT_TRUE(xy.has_value());
    const size_t comma = xy->find(',');
    ASSERT_NE(comma, std::string::npos);
    const double xPx = std::stod(xy->substr(0, comma)) * DPI;
    const double yPx = (score->style().styleD(Sid::pageHeight) - std::stod(xy->substr(comma + 1))) * DPI;

    const RectF box = dolce->pageBoundingRect();
    const double tolerance = 0.05 * dolce->spatium();
    EXPECT_NEAR(xPx, box.center().x(), tolerance);
    EXPECT_NEAR(yPx, box.center().y(), tolerance);

    delete score;
}

TEST_F(Mei_Tests, pianomania_graces_do_not_duplicate_parent_segment_controls) {
    MasterScore* score = ScoreRW::readScore(MEI_DIR + u"pianomania-grace-same-pitch-index.mscx", false);
    ASSERT_TRUE(score);

    std::string meiText;
    ASSERT_TRUE(pmWriteMeiToString(score, true, meiText));
    delete score;

    std::vector<std::string> ids;
    const std::string needle = "xml:id=\"";
    size_t cursor = 0;
    while ((cursor = meiText.find(needle, cursor)) != std::string::npos) {
        cursor += needle.size();
        const size_t end = meiText.find('"', cursor);
        ASSERT_NE(end, std::string::npos);
        ids.push_back(meiText.substr(cursor, end - cursor));
        cursor = end + 1;
    }

    ASSERT_FALSE(ids.empty());
    const std::set<std::string> uniqueIds(ids.begin(), ids.end());
    EXPECT_EQ(uniqueIds.size(), ids.size());
    const std::vector<std::string> dynamicTags = collectStartTags(meiText, "dynam");
    ASSERT_EQ(dynamicTags.size(), 1u);

    const std::vector<std::string> directiveTags = collectStartTags(meiText, "dir");
    ASSERT_EQ(directiveTags.size(), 2u);
    EXPECT_NE(meiText.find(">dolce</dir>"), std::string::npos);
    EXPECT_NE(meiText.find(">legato</dir>"), std::string::npos);
    for (const std::string& directiveTag : directiveTags) {
        EXPECT_TRUE(xmlAttributeValue(directiveTag, "startid").has_value());
        EXPECT_TRUE(xmlAttributeValue(directiveTag, "yOffset").has_value());
        EXPECT_TRUE(xmlAttributeValue(directiveTag, "pm:xy").has_value());
    }

    const std::vector<std::string> noteTags = collectStartTags(meiText, "note");
    ASSERT_EQ(noteTags.size(), 4u);
    const std::optional<std::string> parentId = xmlAttributeValue(noteTags[1], "xml:id");
    ASSERT_TRUE(parentId.has_value());
    EXPECT_EQ(xmlAttributeValue(dynamicTags[0], "startid"), "#" + parentId.value());

    const std::vector<std::string> fingeringTags = collectStartTags(meiText, "fing");
    ASSERT_EQ(fingeringTags.size(), 1u);
    const std::optional<std::string> graceId = xmlAttributeValue(noteTags[3], "xml:id");
    ASSERT_TRUE(graceId.has_value());
    EXPECT_EQ(xmlAttributeValue(fingeringTags[0], "startid"), "#" + graceId.value());
}

TEST_F(Mei_Tests, mei_reh_01) {
    meiReadTest("reh-01");
}

TEST_F(Mei_Tests, mei_roman_numeral_01) {
    meiReadTest("roman-numeral-01");
}

TEST_F(Mei_Tests, mei_score_01) {
    meiReadTest("score-01");
}

TEST_F(Mei_Tests, mei_score_02) {
    meiReadTest("score-02");
}

TEST_F(Mei_Tests, mei_score_03) {
    meiReadTest("score-03");
}

TEST_F(Mei_Tests, mei_slur_01) {
    meiReadTest("slur-01");
}

TEST_F(Mei_Tests, mei_slur_02) {
    meiReadTest("slur-02");
}

TEST_F(Mei_Tests, mei_stem_01) {
    meiReadTest("stem-01");
}

TEST_F(Mei_Tests, mei_tempo_01) {
    meiReadTest("tempo-01");
}

TEST_F(Mei_Tests, mei_tie_01) {
    meiReadTest("tie-01");
}

TEST_F(Mei_Tests, mei_time_signature_01) {
    meiReadTest("time-signature-01");
}

TEST_F(Mei_Tests, mei_time_signature_02) {
    meiReadTest("time-signature-02");
}

TEST_F(Mei_Tests, mei_transpose_01) {
    meiReadTest("transpose-01");
}

TEST_F(Mei_Tests, mei_trill_01) {
    meiReadTest("trill-01");
}

TEST_F(Mei_Tests, mei_tuplet_01) {
    meiReadTest("tuplet-01");
}

TEST_F(Mei_Tests, mei_tuplet_02) {
    meiReadTest("tuplet-02");
}

TEST_F(Mei_Tests, mei_tuplet_03) {
    meiReadTest("tuplet-03");
}

// MEI ending labels are display text, not MuseScore rich-text markup.
TEST_F(Mei_Tests, mei_ending_label_exports_plain_display_text) {
    MasterScore* score = ScoreRW::readScore(MEI_DIR + u"ending-01.mscx", false);
    ASSERT_TRUE(score);
    size_t count = 0;
    for (const auto& entry : score->spannerMap().map()) {
        if (entry.second->isVolta()) {
            toVolta(entry.second)->setText(u"<font face=\"Times New Roman\"></font><b>1.</b>");
            ++count;
        }
    }
    ASSERT_GT(count, 0u);
    score->setLayoutAll();
    score->doLayout();
    const String outputName = u"pianomania-ending-label.test.mei";
    ASSERT_TRUE(ScoreRW::saveScore(score, outputName, [](Score* value, const muse::io::path_t& path) -> Err {
        MeiWriter writer;
        return writer.writeScore(value, path);
    }));
    delete score;
    const auto endings = collectStartTags(readTestTextFile(outputName), "ending");
    ASSERT_FALSE(endings.empty());
    for (const auto& ending : endings) {
        EXPECT_EQ(xmlAttributeValue(ending, "label"), "1.");
    }
}

TEST_F(Mei_Tests, pianomania_staff_centered_dynamic_clears_ledger_accidental) {
    // A Practice export must not move a staff-centred dynamic through an
    // accidental protruding into the grand-staff gap after normal autoplace.
    PianomaniaPrettifyFlagScope flags(true, true);
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(MEI_DIR + u"hairpin-01.mscx", false));
    ASSERT_TRUE(score);
    Segment* segment = score->firstSegment(SegmentType::ChordRest);
    ASSERT_TRUE(segment);
    Chord* chord = toChord(segment->element(0));
    ASSERT_TRUE(chord);
    Note* note = chord->upNote();
    ASSERT_TRUE(note);
    note->setPitch(54, 20, 20); // F-sharp in the lower ledger region of the treble staff.
    Dynamic* dynamic = Factory::createDynamic(segment, true);
    dynamic->setTrack(0);
    dynamic->setDynamicType(DynamicType::P);
    dynamic->setCenterBetweenStaves(AutoOnOff::ON);
    dynamic->setVoiceAssignment(VoiceAssignment::ALL_VOICE_IN_INSTRUMENT);
    segment->add(dynamic);
    score->style().set(Sid::dynamicsHairpinsAutoCenterOnGrandStaff, true);
    score->setLayoutAll();
    score->doLayout();
    ASSERT_TRUE(note->accidental());
    // Align the horizontal attack column while retaining normal vertical
    // autoplace, as a left-offset dynamic beside a ledger accidental does.
    dynamic->setOffset(dynamic->offset() + PointF(note->accidental()->pageBoundingRect().center().x() - dynamic->pageBoundingRect().center().x(), 0.0));
    score->setLayoutAll();
    score->doLayout();
    const RectF accidentalBounds = note->accidental()->pageBoundingRect();
    const RectF dynamicBounds = dynamic->pageBoundingRect();
    ASSERT_LT(dynamicBounds.left(), accidentalBounds.right());
    ASSERT_GT(dynamicBounds.right(), accidentalBounds.left());
    EXPECT_GE(dynamicBounds.top() - accidentalBounds.bottom(), dynamic->minDistance().toMM(dynamic->spatium()) - 0.02 * dynamic->spatium());
}


namespace {
const String PRACTICE_GEOMETRY_FIXTURE = u"pianomania-ornament-cue-keysig-geometry.mscx";

MasterScore* readLaidOutPracticeGeometryFixture()
{
    MasterScore* score = ScoreRW::readScore(MEI_DIR + PRACTICE_GEOMETRY_FIXTURE, false);
    if (score) {
        score->setLayoutAll();
        score->doLayout();
    }
    return score;
}

std::string exportPracticeGeometryFixture(MasterScore* score, const String& outputName)
{
    auto exportFunc = [](Score* current, const muse::io::path_t& path) -> Err {
        MeiWriter meiWriter;
        return meiWriter.writeScore(current, path);
    };
    EXPECT_TRUE(ScoreRW::saveScore(score, outputName, exportFunc));
    return readTestTextFile(outputName);
}

std::array<double, 2> pageInches(const Score* score, const PointF& pagePoint)
{
    return { pagePoint.x() / DPI, score->style().styleD(Sid::pageHeight) - pagePoint.y() / DPI };
}

std::vector<double> parseNumbers(const std::string& value)
{
    std::vector<double> numbers;
    std::string item;
    for (char ch : value + ",") {
        if (ch == ',' || ch == ';') {
            numbers.push_back(std::strtod(item.c_str(), nullptr));
            item.clear();
        } else {
            item += ch;
        }
    }
    return numbers;
}

std::string expectedKeySigSymbols(const KeySig* keySig)
{
    std::string symbols;
    for (const KeySym& keySym : keySig->ldata()->keySymbols) {
        if (!symbols.empty()) {
            symbols += ' ';
        }
        const muse::AsciiStringView name = SymNames::nameForSymId(keySym.sym);
        char xPos[32];
        std::snprintf(xPos, sizeof(xPos), "%.3f", keySym.xPos);
        symbols += std::string(name.ascii(), name.size()) + ":" + xPos + ":" + std::to_string(keySym.line);
    }
    return symbols;
}
}

// Test value: A trill line exports what it draws: the "tr" glyph centre and the
// wavy line, not the never-drawn helper ornament; plain ornaments export
// their glyph centre.
TEST_F(Mei_Tests, mei_export_trill_and_ornaments_carry_drawn_glyph_geometry) {
    MasterScore* score = readLaidOutPracticeGeometryFixture();
    ASSERT_TRUE(score);

    const Trill* trill = nullptr;
    for (const auto& entry : score->spannerMap().map()) {
        if (entry.second && entry.second->isTrill()) {
            trill = toTrill(entry.second);
        }
    }
    ASSERT_TRUE(trill);
    ASSERT_EQ(trill->nsegments(), 1u);
    const TrillSegment* segment = toTrillSegment(trill->frontSegment());
    const SymIdList& symbols = segment->symbols();
    ASSERT_GE(symbols.size(), 2u);
    EXPECT_EQ(symbols.front(), SymId::ornamentTrill);
    const RectF glyph = segment->symBbox(symbols.front()).translated(segment->pagePos());
    const SymIdList wavySymbols(symbols.begin() + 1, symbols.end());
    const double penX = score->engravingFont()->advance(symbols.front(), segment->magS());
    const RectF wavy = segment->symBbox(wavySymbols).translated(segment->pagePos() + PointF(penX, 0.0));
    const auto glyphCenter = pageInches(score, glyph.center());
    const auto wavyStart = pageInches(score, PointF(wavy.left(), wavy.center().y()));
    const auto wavyEnd = pageInches(score, PointF(wavy.right(), wavy.center().y()));
    const Note* trillNote = toChord(trill->startElement())->notes().front();
    const double expectedYOffset = (trillNote->pagePos().y() - glyph.center().y()) / trillNote->spatium();

    std::vector<std::array<double, 2>> ornaments;
    for (Segment* seg = score->firstSegment(SegmentType::ChordRest); seg; seg = seg->next1(SegmentType::ChordRest)) {
        const EngravingItem* item = seg->element(0);
        if (!item || !item->isChord()) {
            continue;
        }
        for (const Articulation* articulation : toChord(item)->articulations()) {
            if (articulation->isOrnament()) {
                const RectF bounds = articulation->symBbox(articulation->symId()).translated(articulation->pagePos());
                ornaments.push_back(pageInches(score, bounds.center()));
            }
        }
    }
    ASSERT_EQ(ornaments.size(), 2u);

    const std::string mei = exportPracticeGeometryFixture(score, u"pianomania-ornament-geometry.test.mei");
    delete score;

    std::vector<std::string> trillTags;
    for (const std::string& tag : collectStartTags(mei, "trill ")) {
        if (xmlAttributeValue(tag, "extender") == "true") {
            trillTags.push_back(tag);
        }
    }
    ASSERT_EQ(trillTags.size(), 1u);
    const std::string& tag = trillTags.front();
    const std::vector<double> xy = parseNumbers(xmlAttributeValue(tag, "pm:xy").value_or(""));
    ASSERT_EQ(xy.size(), 2u) << tag;
    EXPECT_NEAR(xy[0], glyphCenter[0], 0.0006) << tag;
    EXPECT_NEAR(xy[1], glyphCenter[1], 0.0006) << tag;
    const std::vector<double> line = parseNumbers(xmlAttributeValue(tag, "pm:x1y1x2y2").value_or(""));
    ASSERT_EQ(line.size(), 4u) << tag;
    EXPECT_NEAR(line[0], wavyStart[0], 0.0006);
    EXPECT_NEAR(line[1], wavyStart[1], 0.0006);
    EXPECT_NEAR(line[2], wavyEnd[0], 0.0006);
    EXPECT_NEAR(line[3], wavyEnd[1], 0.0006);
    EXPECT_GT(line[0], xy[0]);
    EXPECT_GT(line[2], line[0]);
    EXPECT_EQ(xmlAttributeValue(tag, "pm:segments"), xmlAttributeValue(tag, "pm:x1y1x2y2"));
    EXPECT_NEAR(xmlAttributeDouble(tag, "yOffset").value_or(-99.0), expectedYOffset, 0.051) << tag;

    std::vector<std::string> ornamentTags = collectStartTags(mei, "mordent ");
    ASSERT_EQ(ornamentTags.size(), ornaments.size());
    for (size_t i = 0; i < ornamentTags.size(); ++i) {
        const std::vector<double> ornamentXY = parseNumbers(xmlAttributeValue(ornamentTags[i], "pm:xy").value_or(""));
        ASSERT_EQ(ornamentXY.size(), 2u) << ornamentTags[i];
        EXPECT_NEAR(ornamentXY[0], ornaments[i][0], 0.0006) << ornamentTags[i];
        EXPECT_NEAR(ornamentXY[1], ornaments[i][1], 0.0006) << ornamentTags[i];
        EXPECT_TRUE(xmlAttributeValue(ornamentTags[i], "yOffset").has_value());
    }
}

// Test value: Small (cue-size) notes, chords and rests are marked @cue so the
// Practice renderer draws them, and their accidentals, at cue size.
TEST_F(Mei_Tests, mei_export_small_notes_chords_and_rests_are_cue) {
    MasterScore* score = readLaidOutPracticeGeometryFixture();
    ASSERT_TRUE(score);
    const std::string mei = exportPracticeGeometryFixture(score, u"pianomania-cue.test.mei");
    delete score;

    size_t cueNotes = 0;
    size_t plainNotes = 0;
    for (const std::string& tag : collectStartTags(mei, "note ")) {
        if (xmlAttributeValue(tag, "cue") == "true") {
            ++cueNotes;
        } else {
            EXPECT_FALSE(xmlAttributeValue(tag, "cue").has_value()) << tag;
            ++plainNotes;
        }
    }
    EXPECT_EQ(cueNotes, 4u);
    EXPECT_EQ(plainNotes, 4u);

    const std::vector<std::string> chords = collectStartTags(mei, "chord ");
    ASSERT_EQ(chords.size(), 1u);
    EXPECT_EQ(xmlAttributeValue(chords.front(), "cue"), "true");

    size_t cueRests = 0;
    for (const std::string& tag : collectStartTags(mei, "rest ")) {
        cueRests += xmlAttributeValue(tag, "cue") == "true" ? 1 : 0;
    }
    EXPECT_EQ(cueRests, 1u);
}

// Test value: Key signatures export the symbols MuseScore laid out, including
// the cancellation naturals of a system-end courtesy key change.
TEST_F(Mei_Tests, mei_export_key_signature_symbols_match_layout) {
    MasterScore* score = readLaidOutPracticeGeometryFixture();
    ASSERT_TRUE(score);

    Measure* first = score->firstMeasure();
    ASSERT_TRUE(first && first->nextMeasure());
    ASSERT_NE(first->system(), first->nextMeasure()->system());
    const Segment* announce = first->findSegmentR(SegmentType::KeySigAnnounce, first->ticks());
    ASSERT_TRUE(announce);
    const std::string courtesyTop = expectedKeySigSymbols(toKeySig(announce->element(0)));
    const std::string courtesyBottom = expectedKeySigSymbols(toKeySig(announce->element(VOICES)));
    const Segment* initial = first->findSegment(SegmentType::KeySig, first->tick());
    ASSERT_TRUE(initial);
    const std::string initialTop = expectedKeySigSymbols(toKeySig(initial->element(0)));
    const Segment* change = first->nextMeasure()->findSegment(SegmentType::KeySig, first->nextMeasure()->tick());
    ASSERT_TRUE(change);
    const std::string changeTop = expectedKeySigSymbols(toKeySig(change->element(0)));

    const std::string mei = exportPracticeGeometryFixture(score, u"pianomania-keysig-symbols.test.mei");
    delete score;

    EXPECT_EQ(std::count(courtesyTop.begin(), courtesyTop.end(), ' ') + 1, 8);
    EXPECT_NE(courtesyTop.find("accidentalNatural:0.000:"), std::string::npos);
    EXPECT_NE(courtesyTop.find("accidentalFlat:"), std::string::npos);

    const std::vector<std::string> trailers = collectStartTags(mei, "systemTrailer");
    ASSERT_EQ(trailers.size(), 1u);
    EXPECT_EQ(xmlAttributeValue(trailers.front(), "pm:keysig-symbols-top"), courtesyTop);
    EXPECT_EQ(xmlAttributeValue(trailers.front(), "pm:keysig-symbols-bottom"), courtesyBottom);

    bool sawInitial = false;
    for (const std::string& tag : collectStartTags(mei, "staffDef ")) {
        if (xmlAttributeValue(tag, "n") == "1" && xmlAttributeValue(tag, "keysig") == "4s") {
            EXPECT_EQ(xmlAttributeValue(tag, "pm:keysig-symbols"), initialTop) << tag;
            sawInitial = true;
        }
    }
    EXPECT_TRUE(sawInitial);

    bool sawChange = false;
    for (const std::string& tag : collectStartTags(mei, "scoreDef ")) {
        if (xmlAttributeValue(tag, "keysig") == "4f") {
            EXPECT_EQ(xmlAttributeValue(tag, "pm:keysig-symbols-top"), changeTop) << tag;
            sawChange = true;
        }
    }
    EXPECT_TRUE(sawChange);
}


namespace {
const String V32_REVIEW_FIXTURE = u"pianomania-v32-review-geometry.mscx";

MasterScore* readLaidOutV32ReviewFixture()
{
    MasterScore* score = ScoreRW::readScore(MEI_DIR + V32_REVIEW_FIXTURE, false);
    if (score) {
        score->setLayoutAll();
        score->doLayout();
    }
    return score;
}

// Smallest Euclidean gap between a rect and an item's shape, page coordinates.
double pageGapToShape(const RectF& rect, const EngravingItem* item)
{
    double gap = DBL_MAX;
    // shape() returns a temporary; keep it alive for the loop.
    const Shape shape = item->shape().translated(item->pagePos());
    for (const ShapeElement& element : shape.elements()) {
        const double dx = std::max({ 0.0, element.left() - rect.right(), rect.left() - element.right() });
        const double dy = std::max({ 0.0, element.top() - rect.bottom(), rect.top() - element.bottom() });
        gap = std::min(gap, std::hypot(dx, dy));
    }
    return gap;
}
}

// Scarlatti K. 34 m8: the source hides the grace G#'s sharp. A hidden
// accidental is not engraved, so the export keeps only the alteration
// (@accid.ges) and no drawable @accid or position.
TEST_F(Mei_Tests, mei_export_hidden_accidental_is_gestural_only) {
    std::unique_ptr<MasterScore> score(readLaidOutV32ReviewFixture());
    ASSERT_TRUE(score);
    Chord* chord = toChord(score->firstSegment(SegmentType::ChordRest)->element(0));
    ASSERT_TRUE(chord);
    ASSERT_TRUE(chord->upNote()->accidental());
    ASSERT_FALSE(chord->upNote()->accidental()->visible());

    const std::string meiText = exportPracticeGeometryFixture(score.get(), u"v32-hidden-accidental.test.mei");
    const std::vector<std::string> accids = collectStartTags(meiText, "accid");
    ASSERT_FALSE(accids.empty());
    const std::string& hidden = accids.front();
    EXPECT_EQ(xmlAttributeValue(hidden, "accid.ges"), std::optional<std::string>("s")) << hidden;
    EXPECT_FALSE(xmlAttributeValue(hidden, "accid").has_value()) << hidden;
    EXPECT_FALSE(xmlAttributeValue(hidden, "pm:xy").has_value()) << hidden;
}

// Promenade m51: a source can store a clef change at a system start in the new
// system's header clef. It is the only record of the change, so it is exported
// as a beat-1 clef; unchanged header clefs stay implicit.
TEST_F(Mei_Tests, mei_export_header_clef_change_at_system_start) {
    std::unique_ptr<MasterScore> score(readLaidOutV32ReviewFixture());
    ASSERT_TRUE(score);
    const Measure* third = score->firstMeasure()->nextMeasure()->nextMeasure();
    ASSERT_TRUE(third);
    ASSERT_NE(third->system(), score->firstMeasure()->system()) << "fixture measure 3 must start a system";

    const std::string meiText = exportPracticeGeometryFixture(score.get(), u"v32-header-clef.test.mei");
    size_t staffTwoTreble = 0;
    size_t staffOneChanges = 0;
    for (const std::string& tag : collectStartTags(meiText, "clef")) {
        if (!xmlAttributeValue(tag, "beat").has_value()) {
            continue; // staffDef clefs
        }
        if (xmlAttributeValue(tag, "staff") == std::optional<std::string>("2")) {
            EXPECT_EQ(xmlAttributeValue(tag, "shape"), std::optional<std::string>("G")) << tag;
            EXPECT_EQ(xmlAttributeValue(tag, "beat"), std::optional<std::string>("1.0000")) << tag;
            ++staffTwoTreble;
        } else {
            ++staffOneChanges;
        }
    }
    EXPECT_EQ(staffTwoTreble, 1u);
    EXPECT_EQ(staffOneChanges, 0u);
}

// Moonlight m31: a rest moved into the lower staff from the upper staff must
// clear the lower staff's chord at the same moment instead of sitting on it.
TEST_F(Mei_Tests, pianomania_cross_staff_rest_clears_destination_chord) {
    std::unique_ptr<MasterScore> score(readLaidOutV32ReviewFixture());
    ASSERT_TRUE(score);
    Segment* segment = score->firstSegment(SegmentType::ChordRest);
    ASSERT_TRUE(segment);
    EngravingItem* restItem = segment->element(1); // staff 1, voice 2
    ASSERT_TRUE(restItem && restItem->isRest());
    Rest* rest = toRest(restItem);
    ASSERT_EQ(rest->staffMove(), 1);
    EngravingItem* chordItem = segment->element(VOICES); // staff 2, voice 1
    ASSERT_TRUE(chordItem && chordItem->isChord());
    const Note* top = toChord(chordItem)->upNote();

    const RectF restRect = rest->pageBoundingRect();
    const RectF noteRect = top->pageBoundingRect();
    ASSERT_LT(restRect.left(), noteRect.right());
    ASSERT_GT(restRect.right(), noteRect.left());
    EXPECT_GE(noteRect.top() - restRect.bottom(), 0.3 * rest->spatium());
}

// Moonlight m5: a dynamic centred between the staves keeps half a space from a
// cross-staff beam in the gap; Practice draws the dynamic glyph slightly deeper
// than MuseScore, so a 0.3sp gap that MuseScore left there read as touching.
TEST_F(Mei_Tests, pianomania_staff_centered_dynamic_keeps_cross_staff_beam_margin) {
    PianomaniaPrettifyFlagScope flags(true, true);
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(MEI_DIR + V32_REVIEW_FIXTURE, false));
    ASSERT_TRUE(score);
    Segment* segment = score->firstMeasure()->nextMeasure()->first(SegmentType::ChordRest);
    ASSERT_TRUE(segment);
    Dynamic* dynamic = Factory::createDynamic(segment, true);
    dynamic->setTrack(0);
    dynamic->setDynamicType(DynamicType::PP);
    dynamic->setXmlText(Dynamic::dynamicText(DynamicType::PP));
    dynamic->setCenterBetweenStaves(AutoOnOff::ON);
    dynamic->setVoiceAssignment(VoiceAssignment::ALL_VOICE_IN_INSTRUMENT);
    segment->add(dynamic);
    score->style().set(Sid::dynamicsHairpinsAutoCenterOnGrandStaff, true);
    score->setLayoutAll();
    score->doLayout();

    const Chord* chord = toChord(segment->element(0));
    ASSERT_TRUE(chord && chord->beam() && chord->beam()->cross());
    const Beam* beam = chord->beam();
    const double sp = dynamic->spatium();

    // Seat the dynamic just above the middle of the beam, where the Moonlight
    // "pp" sat.
    const RectF beamBounds = beam->pageBoundingRect();
    dynamic->mutldata()->moveX(beamBounds.center().x() - dynamic->pageBoundingRect().center().x());
    const RectF dynamicRect = dynamic->pageBoundingRect();
    double beamTop = DBL_MAX;
    const Shape beamShape = beam->shape().translated(beam->pagePos());
    for (const ShapeElement& element : beamShape.elements()) {
        if (element.right() > dynamicRect.left() && element.left() < dynamicRect.right()) {
            beamTop = std::min(beamTop, element.top());
        }
    }
    ASSERT_LT(beamTop, DBL_MAX) << "the beam must pass under the dynamic";
    dynamic->mutldata()->moveY(beamTop - 0.3 * sp - dynamicRect.bottom());
    const double seatedGap = pageGapToShape(dynamic->pageBoundingRect(), beam);
    ASSERT_GT(seatedGap, 0.2 * sp);
    ASSERT_LT(seatedGap, 0.45 * sp);

    mu::engraving::rendering::score::SystemLayout::clearStaffCenteredItemsOfNotation({ dynamic }, chord->measure()->system());

    EXPECT_GE(pageGapToShape(dynamic->pageBoundingRect(), beam), 0.5 * sp - 0.02 * sp);
}

// Arabesque m32 "f risoluto": an expression after a dynamic shares its
// baseline even when the source unsnapped it, and the export carries the
// word's left baseline origin so a reader with a narrower italic still starts
// it where MuseScore did (Ave Maria m1 "p religioso").
TEST_F(Mei_Tests, mei_export_snapped_expression_carries_text_origin) {
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(MEI_DIR + u"hairpin-01.mscx", false));
    ASSERT_TRUE(score);
    Segment* first = score->firstSegment(SegmentType::ChordRest);
    ASSERT_TRUE(first);
    Dynamic* dynamic = Factory::createDynamic(first, true);
    dynamic->setTrack(0);
    dynamic->setDynamicType(DynamicType::F);
    first->add(dynamic);
    Expression* snapped = Factory::createExpression(first, true);
    snapped->setTrack(0);
    snapped->setXmlText(u"risoluto");
    snapped->setSnapToDynamics(false);
    first->add(snapped);
    Segment* later = first->next1(SegmentType::ChordRest);
    while (later && later->measure() == first->measure()) {
        later = later->next1(SegmentType::ChordRest);
    }
    ASSERT_TRUE(later);
    Expression* lone = Factory::createExpression(later, true);
    lone->setTrack(0);
    lone->setXmlText(u"dolce");
    later->add(lone);

    mu::engraving::pm::applyPianomaniaAutoLayout(score.get());
    EXPECT_TRUE(snapped->snapToDynamics());
    ASSERT_EQ(snapped->ldata()->itemSnappedBefore(), dynamic);

    const std::array<double, 2> left = pageInches(score.get(), snapped->pageBoundingRect().topLeft());
    const std::array<double, 2> bottom = pageInches(score.get(), snapped->pageBoundingRect().bottomLeft());
    const std::string meiText = exportPracticeGeometryFixture(score.get(), u"v32-text-origin.test.mei");
    std::optional<std::string> snappedOrigin;
    bool loneHasOrigin = false;
    for (const std::string& tag : collectStartTags(meiText, "dir")) {
        const std::optional<std::string> origin = xmlAttributeValue(tag, "pm:text-origin");
        const size_t textStart = meiText.find(tag) + tag.size();
        if (meiText.compare(textStart, 8, "risoluto") == 0) {
            snappedOrigin = origin;
        } else if (meiText.compare(textStart, 5, "dolce") == 0) {
            loneHasOrigin = origin.has_value();
        }
    }
    ASSERT_TRUE(snappedOrigin.has_value());
    EXPECT_FALSE(loneHasOrigin);
    const std::vector<double> xy = parseNumbers(*snappedOrigin);
    ASSERT_EQ(xy.size(), 2u);
    const double spInches = snapped->spatium() / DPI;
    EXPECT_NEAR(xy[0], left[0], 0.3 * spInches);
    EXPECT_LE(xy[1], left[1]);
    EXPECT_GE(xy[1], bottom[1]);
}

}
