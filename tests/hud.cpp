#include "PlayerFixture.h"
#include "rp/Hud.h"
#include "rp/PortView.h"
#include <iostream>
#include <stdexcept>
#define CHECK(x)                                                                                   \
    do {                                                                                           \
        if (!(x))                                                                                  \
            throw std::runtime_error(#x);                                                          \
    } while (false)

namespace rp {
    struct PortViewTestAccess {
        static auto& menu(PortView& v) {
            return *v.xmlMenu_;
        }

        static std::string name(PortView& v) {
            return v.xmlName_;
        }

        static void openShop(PortView& v, const Game& g, const std::string& name) {
            v.panel_ = PortView::Panel::Shop;
            v.openXml(g, name);
        }

        static size_t page(const PortView& v) {
            return v.page_;
        }

        static int selection(const PortView& v) {
            return v.shopSelection_;
        }

        static size_t itemCount(const PortView& v) {
            return v.xmlItems_.size();
        }

        static float listTop(const PortView& v) {
            return v.blueprintListTop_;
        }
    };
} // namespace rp

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: raftpirates_hud_tests ASSET_ROOT\n";
        return 2;
    }
    try {
        rp::Game g;
        loadPlayerFixture(g);
        g.daring = 4;
        g.daringTotal = 120;
        g.daringRemaining = 60;
        CHECK(g.daringLocked() && g.daringProgress() == 0 && !g.canAdvanceDaring());
        rp::Hud hud(argv[1]);
        hud.update(g);
        const auto test = [&](const char* name, rp::Hud::Action action) {
            auto bounds = hud.bounds(name);
            CHECK(bounds.w > 0 && bounds.h > 0);
            CHECK(hud.touch(bounds.x + bounds.w / 2, bounds.y + bounds.h / 2) == action);
        };
        test("QuestMenu", rp::Hud::Action::Quest);
        test("SafeFlag", rp::Hud::Action::Safety);
        test("DaringMeter", rp::Hud::Action::Daring);
        test("EventNotification", rp::Hud::Action::ToggleView);
        test("MainMenu", rp::Hud::Action::Shop);
        test("PirateMap", rp::Hud::Action::Map);
        g.safe = false;
        g.badZone = true;
        hud.update(g);
        test("BadZoneFlag", rp::Hud::Action::Safety);
        sys::gfx::Image img(640, 400);
        hud.render(img);
        CHECK(img.at(320, 30).a > 0);
        rp::PortView view(argv[1]);
        g.badZone = false;
        CHECK(!g.daringLocked() && g.daringProgress() == .5 && !g.canAdvanceDaring());
        g.daringRemaining = 0;
        CHECK(g.canAdvanceDaring());
        auto bounds = hud.bounds("DaringMeter");
        auto req = view.touch(g, bounds.x + bounds.w / 2, bounds.y + bounds.h / 2);
        CHECK(req && req->payload.getString("c") == "daring_advance");
        // Original Daring popup should open its matching store category, not send a purchase.
        g.daringRemaining = 60;
        rp::StoreItem daring;
        daring.id = 101;
        daring.name = "daring";
        daring.group = "misc";
        daring.currency = 4;
        daring.price = 10;
        daring.title = "DARING_METER_DESC";
        g.storeItems.emplace(daring.id, daring);
        rp::PortView shop(argv[1]);
        shop.touch(g, bounds.x + bounds.w / 2, bounds.y + bounds.h / 2);
        shop.render(g, 0, "");
        CHECK(rp::PortViewTestAccess::name(shop) == "daring_popup");
        auto& daringMenu = rp::PortViewTestAccess::menu(shop);
        auto mainButton = daringMenu.find(1, "MainButton", true);
        CHECK(mainButton && daringMenu.node(mainButton).number("gotostore") == 1);
        auto touchId = daringMenu.find(mainButton, "Touch");
        auto touchBounds = daringMenu.node(touchId).bounds;
        shop.touch(g, touchBounds.x + touchBounds.w / 2, touchBounds.y + touchBounds.h / 2);
        CHECK(rp::PortViewTestAccess::name(shop) == "tabbed_menu_misc");
        auto& shopMenu = rp::PortViewTestAccess::menu(shop);
        auto frame = shopMenu.find(1, "Frame", true);
        CHECK(frame && shopMenu.isVisible(frame));
        auto price = shopMenu.find(frame, "PriceText");
        CHECK(price && shopMenu.node(price).text("text") == "10");
        shop.render(g, 0, "");
        auto item = shopMenu.find(1, "tabbedItem0", true);
        CHECK(item);
        auto itemTouch = shopMenu.find(item, "Touch");
        auto itemBounds = shopMenu.node(itemTouch).bounds;
        CHECK(itemBounds.w > 0 && itemBounds.h > 0);
        CHECK(shopMenu.hitTest(itemBounds.x + itemBounds.w / 2, itemBounds.y + itemBounds.h / 2) ==
              itemTouch);
        shop.touchDown(itemBounds.x + itemBounds.w / 2, itemBounds.y + itemBounds.h / 2);
        shop.touch(g, itemBounds.x + itemBounds.w / 2, itemBounds.y + itemBounds.h / 2);
        CHECK(shopMenu.isVisible(frame));
        for (int tab = 1; tab <= 4; ++tab)
            if (auto id = shopMenu.find(1, "Tab" + std::to_string(tab), true)) {
                const auto b = shopMenu.node(id).bounds;
                CHECK(b.x >= 0 && b.y >= 0 && b.x + b.w <= 640 && b.h <= 50);
            }
        bounds = hud.bounds("QuestMenu");
        req = view.touch(g, bounds.x + bounds.w / 2, bounds.y + bounds.h / 2);
        CHECK(req && req->payload.getString("c") == "gs_quest");
        view.touch(g, 500, 310);
        bounds = hud.bounds("EventNotification");
        view.touch(g, bounds.x + bounds.w / 2, bounds.y + bounds.h / 2);
        CHECK(view.grid);
        bounds = hud.bounds("PirateMap");
        view.touch(g, bounds.x + bounds.w / 2, bounds.y + bounds.h / 2);
        CHECK(view.map);
        rp::OceanView ocean(argv[1]);
        g.objects.at(3).name = "Diving structure";
        const auto unit = g.GetDiveUnit();
        CHECK(unit.first == 1 && unit.second == 2);
        const auto& comp = g.rafts.front().components.back();
        auto pos = ocean.WorldToScreen({(comp.x + .5f) / 6, (comp.y + .5f) / 6});
        CHECK(ocean.Pick(g, unit.first, unit.second, pos, 0));
        rp::PortView diver(argv[1]);
        diver.render(g, 0, "");
        auto diveRequest = diver.touch(g, pos.x, pos.y);
        CHECK(diveRequest && diveRequest->payload.getString("c") == "dive_start");
        g.diveState.parent = unit.first;
        g.diveState.object = unit.second;
        g.diveState.StartDive(20, 19.7f);
        auto drop = ocean.render(g, 1, false, &g.diveState);
        g.diveState.remaining = 16;
        auto submerged = ocean.render(g, 4, false, &g.diveState);
        size_t changed = 0;
        for (size_t i = 0; i < drop.pixels.size(); ++i) {
            const auto& a = drop.pixels[i];
            const auto& b = submerged.pixels[i];
            if (a.r != b.r || a.g != b.g || a.b != b.b || a.a != b.a)
                ++changed;
        }
        CHECK(changed > 100);
        // Transparent diving panel must preserve the raft/ocean outside its bounds.
        rp::diving_game::DivingGame minigame(argv[1]);
        sys::gfx::Image background(640, 400, {23, 51, 77, 255});
        minigame.render(background);
        const auto corner = background.at(0, 0);
        CHECK(corner.r == 23 && corner.g == 51 && corner.b == 77 && corner.a == 255);
        CHECK(background.at(639, 399).r == 23);
        // Exercise more than one catalogue page, including selection without a full
        // generic touch/layout traversal.
        rp::Game catalogue;
        loadPlayerFixture(catalogue);
        for (int id = 2; id <= 8; ++id) {
            auto entry = catalogue.blueprints.at(1);
            catalogue.blueprints.emplace(id, std::move(entry));
        }
        rp::PortView browser(argv[1]);
        rp::PortViewTestAccess::openShop(browser, catalogue, "tabbed_menu_blueprints");
        CHECK(rp::PortViewTestAccess::itemCount(browser) == 5);
        const float listY = rp::PortViewTestAccess::listTop(browser) + 12;
        int first = rp::PortViewTestAccess::selection(browser);
        browser.touchDown(190, listY);
        browser.touch(catalogue, 190, listY);
        CHECK(rp::PortViewTestAccess::selection(browser) != first);
        browser.touchDown(450, listY);
        browser.touchDrag(100, listY);
        browser.touch(catalogue, 100, listY);
        CHECK(rp::PortViewTestAccess::page(browser) == 1);
        CHECK(rp::PortViewTestAccess::itemCount(browser) == 3);
        std::cout << "Original HUD asset, action, and protocol checks passed\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
