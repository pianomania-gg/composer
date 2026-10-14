#include <gtest/gtest.h>
#include "../internal/authoringcapabilitypolicy.h"
#include "../internal/authoringpolicycore.h"
#include "testing/environment.h"
#include "draw/drawmodule.h"
#include "engraving/engravingmodule.h"
#include "engraving/compat/dummyelement.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/playtechannotation.h"
#include "engraving/dom/score.h"
#include "engraving/dom/symbol.h"
#include "engraving/dom/tremolosinglechord.h"
#include "engraving/dom/tremolotwochord.h"

using namespace mu::notation;
using namespace mu::notation::authoring;
using namespace testing;
using namespace mu::engraving;

static muse::testing::SuiteEnvironment authoringEnvironment(
    { new muse::draw::DrawModule(), new EngravingModule() },
    nullptr,
    []() {
        MScore::testMode = true;
        MScore::noGui = true;
    });

TEST(AuthoringCapabilityPolicy, EffectiveRegistryRetainsUnreviewedPendingRows)
{
    std::size_t hidden = 0;
    std::size_t pending = 0;
    for (const auto& row : capabilityRows) {
        hidden += row.decision == Decision::Hidden;
        pending += row.decision == Decision::ReviewRetained;
    }
    EXPECT_EQ(hidden, 1948u);
    EXPECT_EQ(pending, 2104u);
    EXPECT_EQ(findCapability("template.custom.user")->decision, Decision::ReviewRetained);
    EXPECT_EQ(findCapability("palette.text.playing-technique-type-distortion")->decision, Decision::Hidden);
}

TEST(AuthoringCapabilityPolicy, DedicatedCreationCannotBorrowExistingEditAuthority)
{
    EXPECT_FALSE(allowed(decide("action.dip")));
    EXPECT_FALSE(allowed(decide("action.dip", { Operation::CreateFromDedicatedControl, true })));
    EXPECT_FALSE(allowed(decide("action.edit-strings", { Operation::EditExisting, false })));
    EXPECT_TRUE(allowed(decide("action.edit-strings", { Operation::EditExisting, true })));
    EXPECT_TRUE(allowed(decide("action.dip", { Operation::PreserveImported, false })));
}

TEST(AuthoringCapabilityPolicy, ExactActionsAndInstruments)
{
    AuthoringCapabilityPolicy policy;
    EXPECT_FALSE(policy.mayCreateAction("dip"));
    EXPECT_FALSE(policy.mayCreateAction("insert-fretframe"));
    EXPECT_TRUE(policy.mayCreateAction("chord-text"));
    EXPECT_TRUE(policy.mayCreateAction("unlisted-action"));
    EXPECT_FALSE(policy.mayCreateInstrument("guitar-nylon"));
    EXPECT_TRUE(policy.mayCreateInstrument("piano"));
    EXPECT_FALSE(policy.mayCreateInstrument("harpsichord"));
    EXPECT_FALSE(policy.mayCreateAction("hammer-on-pull-off"));
    EXPECT_TRUE(policy.mayCreateInstrument("electric-piano"));
}

TEST(AuthoringCapabilityPolicy, RawSymbolsUseApprovedExactIdentities)
{
    AuthoringCapabilityPolicy policy;
    EXPECT_FALSE(policy.mayCreateSymbol(mu::engraving::SymId::accdnCombDot));
    EXPECT_TRUE(policy.mayCreateSymbol(mu::engraving::SymId::stringsThumbPosition));
    EXPECT_FALSE(policy.mayCreateSymbol(mu::engraving::SymId::stringsHarmonic));
    EXPECT_FALSE(policy.mayCreateSymbolRange("Accordion"));
    EXPECT_TRUE(policy.mayCreateSymbolRange("Unlisted range"));
    EXPECT_FALSE(policy.mayEditExisting(nullptr, nullptr));
}

TEST(AuthoringCapabilityPolicy, BowCommandsAndSymbolRoutesCannotBypassPaletteRestrictions)
{
    AuthoringCapabilityPolicy policy;
    EXPECT_FALSE(policy.mayCreateAction("add-up-bow"));
    EXPECT_FALSE(policy.mayCreateAction("add-down-bow"));
    for (SymId sym : { SymId::stringsUpBow, SymId::stringsDownBow, SymId::stringsHarmonic,
                       SymId::stringsHalfHarmonic, SymId::pluckedSnapPizzicatoAbove,
                       SymId::pluckedSnapPizzicatoBelow, SymId::wiggleVibratoLargeFaster }) {
        EXPECT_FALSE(policy.mayCreateSymbol(sym));
        auto articulation = Factory::makeArticulation(Score::paletteScore()->dummy()->chord());
        articulation->setSymId(sym);
        EXPECT_FALSE(policy.mayCreateElement(*articulation));
        std::unique_ptr<Symbol> symbol(Factory::createSymbol(Score::paletteScore()->dummy()));
        symbol->setSym(sym);
        EXPECT_FALSE(policy.mayCreateElement(*symbol));
        // Imported editing authority does not depend on creation permissions.
        EXPECT_TRUE(policy.mayEditExisting(articulation.get(), Score::paletteScore()));
        EXPECT_EQ(articulation->symId(), sym);
    }
}

TEST(AuthoringCapabilityPolicy, PianoArticulationsAndFingeringSymbolsRemainAvailable)
{
    AuthoringCapabilityPolicy policy;
    for (SymId sym : { SymId::articAccentAbove, SymId::articStaccatoAbove,
                       SymId::articTenutoAbove, SymId::articStaccatissimoWedgeAbove,
                       SymId::articSoftAccentAbove, SymId::articMarcatoAbove,
                       SymId::stringsThumbPosition, SymId::arpeggiatoUp, SymId::pluckedDamp }) {
        EXPECT_TRUE(policy.mayCreateSymbol(sym));
        auto articulation = Factory::makeArticulation(Score::paletteScore()->dummy()->chord());
        articulation->setSymId(sym);
        EXPECT_TRUE(policy.mayCreateElement(*articulation));
    }
}

TEST(AuthoringCapabilityPolicy, StringTechniquePresetsDoNotHideGenericPianoAnnotations)
{
    AuthoringCapabilityPolicy policy;
    auto annotation = std::make_shared<PlayTechAnnotation>(Score::paletteScore()->dummy()->segment());
    for (PlayingTechniqueType technique : { PlayingTechniqueType::Harmonics, PlayingTechniqueType::Pizzicato,
                                             PlayingTechniqueType::Detache, PlayingTechniqueType::Martele,
                                             PlayingTechniqueType::SulPonticello, PlayingTechniqueType::SulTasto,
                                             PlayingTechniqueType::Vibrato, PlayingTechniqueType::Mute,
                                             PlayingTechniqueType::Open, PlayingTechniqueType::Distortion,
                                             PlayingTechniqueType::JazzTone, PlayingTechniqueType::ColLegno }) {
        annotation->setTechniqueType(technique);
        EXPECT_FALSE(policy.mayCreateElement(*annotation));
        EXPECT_TRUE(policy.mayEditExisting(annotation.get(), Score::paletteScore()));
    }
    for (PlayingTechniqueType technique : { PlayingTechniqueType::Legato, PlayingTechniqueType::Tremolo,
                                             PlayingTechniqueType::Natural }) {
        annotation->setTechniqueType(technique);
        annotation->setXmlText(u"normal");
        EXPECT_TRUE(policy.mayCreateElement(*annotation));
    }
    annotation->setTechniqueType(PlayingTechniqueType::Natural);
    annotation->setXmlText(u"arco");
    EXPECT_FALSE(policy.mayCreateElement(*annotation));
    annotation->setXmlText(u"normal");
    EXPECT_TRUE(policy.mayCreateElement(*annotation));
}

TEST(AuthoringCapabilityPolicy, SpecialistSymbolsCannotBypassHiddenRangesThroughSearchOrSavedPalettes)
{
    AuthoringCapabilityPolicy policy;
    for (std::string_view range : { "Kahnotation", "Daseian notation", "Kievan square notation",
                                    "Medieval and Renaissance plainchant articulations",
                                    "Medieval and Renaissance plainchant multiple-note forms",
                                    "Medieval and Renaissance plainchant single-note forms", "Techniques noteheads" }) {
        EXPECT_FALSE(policy.mayCreateSymbolRange(range));
    }
    for (SymId sym : { SymId::kahnStep, SymId::daseianFinales1, SymId::kievanNoteWhole,
                       SymId::chantPunctum, SymId::chantIctusAbove, SymId::chantPodatusLower,
                       SymId::swissRudimentsNoteheadBlackFlam, SymId::unpitchedPercussionClef1,
                       SymId::semipitchedPercussionClef2, SymId::accdnDiatonicClef,
                       SymId::indianDrumClef, SymId::fingeringPLower,
                       SymId::fingeringQLower, SymId::fingeringSLower,
                       SymId::guitarVibratoStroke, SymId::stemBowOnBridge,
                       SymId::oneHandedRollStevens, SymId::tremoloDivisiDots2 }) {
        EXPECT_FALSE(policy.mayCreateSymbol(sym));
        std::unique_ptr<Symbol> symbol(Factory::createSymbol(Score::paletteScore()->dummy()));
        symbol->setSym(sym);
        EXPECT_FALSE(policy.mayCreateElement(*symbol));
        EXPECT_TRUE(policy.mayEditExisting(symbol.get(), Score::paletteScore()));
    }
    for (SymId sym : { SymId::gClef, SymId::cClef, SymId::fClef, SymId::fingering1,
                       SymId::fingering5, SymId::fingeringSubstitutionAbove, SymId::keyboardPedalPed,
                       SymId::ornamentMordent, SymId::noteheadBlack, SymId::tremolo1 }) {
        EXPECT_TRUE(policy.mayCreateSymbol(sym));
    }
}

TEST(AuthoringCapabilityPolicy, PercussionRollRestrictionsPreserveOrdinaryPianoTremolosAndImportedRolls)
{
    AuthoringCapabilityPolicy policy;
    auto single = Factory::makeTremoloSingleChord(Score::paletteScore()->dummy()->chord());
    single->setTremoloType(TremoloType::BUZZ_ROLL);
    EXPECT_FALSE(policy.mayCreateElement(*single));
    EXPECT_TRUE(policy.mayEditExisting(single.get(), Score::paletteScore()));
    EXPECT_EQ(single->tremoloType(), TremoloType::BUZZ_ROLL);
    EXPECT_FALSE(policy.mayCreateSymbol(SymId::buzzRoll));
    for (TremoloType type : { TremoloType::R8, TremoloType::R16, TremoloType::R32, TremoloType::R64 }) {
        single->setTremoloType(type);
        EXPECT_TRUE(policy.mayCreateElement(*single));
    }
    auto pair = Factory::makeTremoloTwoChord(Score::paletteScore()->dummy()->chord());
    for (TremoloType type : { TremoloType::C8, TremoloType::C16, TremoloType::C32, TremoloType::C64 }) {
        pair->setTremoloType(type);
        EXPECT_TRUE(policy.mayCreateElement(*pair));
    }
}
