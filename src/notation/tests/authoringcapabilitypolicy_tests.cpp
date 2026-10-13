#include <gtest/gtest.h>
#include "../internal/authoringcapabilitypolicy.h"
#include "../internal/authoringpolicycore.h"

using namespace mu::notation;
using namespace mu::notation::authoring;
using namespace testing;

TEST(AuthoringCapabilityPolicy, EffectiveRegistryRetainsOriginalPendingRows)
{
    std::size_t hidden = 0;
    std::size_t pending = 0;
    for (const auto& row : capabilityRows) {
        hidden += row.decision == Decision::Hidden;
        pending += row.decision == Decision::ReviewRetained;
    }
    EXPECT_EQ(hidden, 1675u);
    EXPECT_EQ(pending, 2356u);
    EXPECT_EQ(findCapability("template.custom.user")->decision, Decision::ReviewRetained);
    EXPECT_EQ(findCapability("palette.text.playing-technique-type-distortion")->decision, Decision::ReviewRetained);
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
    EXPECT_TRUE(policy.mayCreateInstrument("harpsichord"));
}

TEST(AuthoringCapabilityPolicy, RawSymbolsUseApprovedExactIdentities)
{
    AuthoringCapabilityPolicy policy;
    EXPECT_FALSE(policy.mayCreateSymbol(mu::engraving::SymId::accdnCombDot));
    EXPECT_TRUE(policy.mayCreateSymbol(mu::engraving::SymId::stringsThumbPosition));
    EXPECT_TRUE(policy.mayCreateSymbol(mu::engraving::SymId::stringsHarmonic));
    EXPECT_FALSE(policy.mayCreateSymbolRange("Accordion"));
    EXPECT_TRUE(policy.mayCreateSymbolRange("Unlisted range"));
    EXPECT_FALSE(policy.mayEditExisting(nullptr, nullptr));
}
