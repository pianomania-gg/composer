// SPDX-License-Identifier: GPL-3.0-only
#include <gtest/gtest.h>

#include <set>
#include <string>

#include "../authoringcreationmenus.h"

using namespace mu::appshell;
using namespace muse;
using namespace muse::uicomponents;

namespace {
class RestrictedActionsPolicy final : public mu::notation::IAuthoringCapabilityPolicy
{
public:
    bool isRestrictedAction(std::string_view code) const override { return m_restricted.count(std::string(code)) != 0; }
    bool mayCreateAction(std::string_view code) const override { return !isRestrictedAction(code); }
    bool mayCreateInstrument(std::string_view) const override { return true; }
    bool mayCreateTemplate(const io::path_t&) const override { return true; }
    bool mayCreateElement(const mu::engraving::EngravingItem&, std::string_view) const override { return true; }
    bool mayCreateSymbol(mu::engraving::SymId) const override { return true; }
    bool mayCreateSymbolRange(std::string_view) const override { return true; }
    bool mayEditExisting(const mu::engraving::EngravingItem*, const mu::engraving::Score*) const override { return false; }

private:
    const std::set<std::string> m_restricted { "toggle-percussion-panel", "add-fretboard-diagram", "insert-fretframe" };
};

MenuItem* actionItem(const char* code, QObject* owner)
{
    ui::UiAction action;
    action.code = code;
    action.title = MnemonicString(TranslatableString::untranslatable(code));
    return new MenuItem(action, owner);
}

MenuItem* separator(QObject* owner)
{
    MenuItem* item = new MenuItem(owner);
    item->setAction(ui::UiAction());
    return item;
}

MenuItem* menu(const char* id, const MenuItemList& subitems, QObject* owner)
{
    MenuItem* item = new MenuItem(owner);
    item->setId(id);
    item->setSubitems(subitems);
    ui::UiAction action;
    action.title = MnemonicString(TranslatableString::untranslatable(id));
    item->setAction(action);
    return item;
}

struct ChangeCounter {
    explicit ChangeCounter(MenuItem* item)
    {
        QObject::connect(item, &MenuItem::subitemsChanged, item, [this]() { ++count; });
    }

    int count = 0;
};

struct Menus {
    QObject owner;
    MenuItem* mixer = actionItem("toggle-mixer", &owner);
    MenuItem* percussion = actionItem("toggle-percussion-panel", &owner);
    MenuItem* viewSeparator = separator(&owner);
    MenuItem* toolbars = menu("menu-toolbars", { actionItem("toggle-noteinput", &owner) }, &owner);
    MenuItem* view = menu("menu-view", { mixer, percussion, viewSeparator, toolbars }, &owner);

    MenuItem* chordText = actionItem("chord-text", &owner);
    MenuItem* fretboard = actionItem("add-fretboard-diagram", &owner);
    MenuItem* chordsSeparator = separator(&owner);
    MenuItem* legend = actionItem("insert-fretframe", &owner);
    MenuItem* chords = menu("menu-chord-and-frets", { chordText, fretboard, chordsSeparator, legend }, &owner);
    MenuItem* add = menu("menu-add", { chords }, &owner);

    MenuItem* fretboardsOnly = menu("menu-fretboards", { actionItem("insert-fretframe", &owner) }, &owner);
    MenuItem* help = menu("menu-help", { actionItem("about-musescore", &owner) }, &owner);
};
}

TEST(AuthoringCreationMenusTests, SelectionsWithTheSamePercussionAnswerLeaveTheMenusAlone)
{
    Menus m;
    AuthoringCreationMenus creationMenus({ m.view, m.add, m.fretboardsOnly, m.help }, std::make_shared<RestrictedActionsPolicy>());

    const auto first = creationMenus.update(false);
    ASSERT_TRUE(first.has_value());
    EXPECT_EQ(*first, (MenuItemList { m.view, m.add, m.help }));
    EXPECT_EQ(m.view->subitems(), (MenuItemList { m.mixer, m.viewSeparator, m.toolbars }));
    EXPECT_EQ(m.chords->subitems(), (MenuItemList { m.chordText, m.chordsSeparator }));

    ChangeCounter viewChanges(m.view);
    ChangeCounter addChanges(m.add);
    ChangeCounter chordsChanges(m.chords);
    ChangeCounter toolbarsChanges(m.toolbars);

    // Clicks, arrow keys and note entry that keep the selection off existing percussion
    EXPECT_FALSE(creationMenus.update(false).has_value());
    EXPECT_FALSE(creationMenus.update(false).has_value());
    EXPECT_EQ(viewChanges.count, 0);

    // Selecting an existing percussion item brings back its percussion editor, and nothing else
    const auto percussion = creationMenus.update(true);
    ASSERT_TRUE(percussion.has_value());
    EXPECT_EQ(*percussion, (MenuItemList { m.view, m.add, m.help }));
    EXPECT_EQ(m.view->subitems(), (MenuItemList { m.mixer, m.percussion, m.viewSeparator, m.toolbars }));
    EXPECT_EQ(viewChanges.count, 1);

    // Moving between existing percussion items
    EXPECT_FALSE(creationMenus.update(true).has_value());
    EXPECT_EQ(viewChanges.count, 1);

    const auto back = creationMenus.update(false);
    ASSERT_TRUE(back.has_value());
    EXPECT_EQ(m.view->subitems(), (MenuItemList { m.mixer, m.viewSeparator, m.toolbars }));
    EXPECT_EQ(viewChanges.count, 2);

    EXPECT_EQ(addChanges.count, 0);
    EXPECT_EQ(chordsChanges.count, 0);
    EXPECT_EQ(toolbarsChanges.count, 0);
}

TEST(AuthoringCreationMenusTests, PercussionAnswerWithoutPercussionEditorsLeavesTheMenusAlone)
{
    Menus m;
    AuthoringCreationMenus creationMenus({ m.add, m.help }, std::make_shared<RestrictedActionsPolicy>());

    ASSERT_TRUE(creationMenus.update(false).has_value());
    ChangeCounter chordsChanges(m.chords);

    EXPECT_FALSE(creationMenus.update(true).has_value());
    EXPECT_FALSE(creationMenus.update(false).has_value());
    EXPECT_EQ(chordsChanges.count, 0);
    EXPECT_EQ(m.chords->subitems(), (MenuItemList { m.chordText, m.chordsSeparator }));
}
