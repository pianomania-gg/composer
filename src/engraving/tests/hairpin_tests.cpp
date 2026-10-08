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

#include "engraving/dom/hairpin.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/barline.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/system.h"

#include "engraving/compat/scoreaccess.h"
#include "utils/scorerw.h"

using namespace mu::engraving;

class Engraving_HairpinTests : public ::testing::Test
{
};

static const String HAIRPIN_DATA_DIR(u"hairpin_data/");

TEST_F(Engraving_HairpinTests, hairpin)
{
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);
    Hairpin* hp = new Hairpin(score->dummy()->segment());

    // subtype
    hp->setHairpinType(HairpinType::DIM_HAIRPIN);
    Hairpin* hp2 = toHairpin(ScoreRW::writeReadElement(hp));
    EXPECT_EQ(hp2->hairpinType(), HairpinType::DIM_HAIRPIN);
    delete hp2;

    hp->setHairpinType(HairpinType::CRESC_HAIRPIN);
    hp2 = toHairpin(ScoreRW::writeReadElement(hp));
    EXPECT_EQ(hp2->hairpinType(), HairpinType::CRESC_HAIRPIN);
    delete hp2;
}

TEST_F(Engraving_HairpinTests, graphicalEndpointClearsFinalBarlineWithoutBlockingInteriorCrossing)
{
    MasterScore* score = ScoreRW::readScore(HAIRPIN_DATA_DIR + "end-barline-clearance.mscx");
    ASSERT_TRUE(score);

    std::vector<Hairpin*> hairpins;
    for (const auto& pair : score->spanner()) {
        if (pair.second && pair.second->isHairpin()) {
            hairpins.push_back(toHairpin(pair.second));
        }
    }
    ASSERT_EQ(hairpins.size(), 3u);

    HairpinSegment* finalBarlineHairpin = toHairpinSegment(hairpins[0]->frontSegment());
    ASSERT_TRUE(finalBarlineHairpin);
    EngravingItem* snappedAfter = finalBarlineHairpin->ldata()->itemSnappedAfter();
    ASSERT_TRUE(snappedAfter);
    ASSERT_TRUE(snappedAfter->isDynamic());
    const double dynamicPageX = snappedAfter->pageX();
    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);
    Segment* firstEndBarlineSegment = firstMeasure->findSegment(SegmentType::EndBarLine, firstMeasure->endTick());
    ASSERT_TRUE(firstEndBarlineSegment);
    BarLine* firstEndBarline = toBarLine(firstEndBarlineSegment->element(0));
    ASSERT_TRUE(firstEndBarline);

    const double spatium = finalBarlineHairpin->spatium();
    const double endpointX = finalBarlineHairpin->pos().x() + finalBarlineHairpin->pos2().x();
    const double barlineLeft = firstEndBarline->pageX() - finalBarlineHairpin->system()->pageX()
                               + firstEndBarline->ldata()->bbox().left();
    EXPECT_GE(barlineLeft - endpointX, 0.5 * spatium - 0.01);

    score->setLayoutAll();
    score->doLayout();
    EXPECT_EQ(finalBarlineHairpin->ldata()->itemSnappedAfter(), snappedAfter);
    EXPECT_NEAR(snappedAfter->pageX(), dynamicPageX, 0.01);

    HairpinSegment* crossingHairpin = toHairpinSegment(hairpins[1]->frontSegment());
    ASSERT_TRUE(crossingHairpin);
    Measure* secondMeasure = firstMeasure->nextMeasure();
    ASSERT_TRUE(secondMeasure);
    Segment* interiorBarlineSegment = secondMeasure->findSegment(SegmentType::EndBarLine, secondMeasure->endTick());
    ASSERT_TRUE(interiorBarlineSegment);
    BarLine* interiorBarline = toBarLine(interiorBarlineSegment->element(0));
    ASSERT_TRUE(interiorBarline);

    const double crossingStart = crossingHairpin->pos().x();
    const double crossingEnd = crossingStart + crossingHairpin->pos2().x();
    const double interiorBarlineX = interiorBarline->pageX() - crossingHairpin->system()->pageX();
    EXPECT_LT(crossingStart, interiorBarlineX);
    EXPECT_GT(crossingEnd, interiorBarlineX);

    HairpinSegment* intentionalExtension = toHairpinSegment(hairpins[2]->frontSegment());
    ASSERT_TRUE(intentionalExtension);
    Measure* fourthMeasure = secondMeasure->nextMeasure()->nextMeasure();
    ASSERT_TRUE(fourthMeasure);
    Segment* fourthEndBarlineSegment = fourthMeasure->findSegment(SegmentType::EndBarLine, fourthMeasure->endTick());
    ASSERT_TRUE(fourthEndBarlineSegment);
    BarLine* fourthEndBarline = toBarLine(fourthEndBarlineSegment->element(0));
    ASSERT_TRUE(fourthEndBarline);
    const double intentionalEnd = intentionalExtension->pos().x() + intentionalExtension->pos2().x();
    const double fourthBarlineRight = fourthEndBarline->pageX() - intentionalExtension->system()->pageX()
                                      + fourthEndBarline->ldata()->bbox().right();
    EXPECT_GT(intentionalEnd, fourthBarlineRight + 0.5 * intentionalExtension->spatium());

    delete score;
}
