#pragma once
#include "rp/Game.h"

inline void loadPlayerFixture(rp::Game& g) {
    g = rp::Game{};
    g.objects[1] = {1, 0, 1000, "CoreRaft", "gfx/rafts/raft_core.png", -192, 0, {{0, 0}}};
    g.objects[2] = {2, 1, 1000,    "boom_buddha", "xml_bin/anim_boom_buddha_robot.bin",
                    0, 0, {{0, 0}}};
    g.objects[3] = {3, 1, 1000, "DivingUnit", "xml_bin/anim_core_dive_unit.bin", 0, 0, {{0, 0}}};
    g.objects[4] = {4,
                    1,
                    1000,
                    "pain_sprinkler",
                    "xml_bin/anim_pain_sprinkler_front.bin",
                    0,
                    0,
                    {{0, 1}, {1, 0}, {1, 1}, {1, 2}, {2, 1}}};
    g.blueprints[1] = {1, 4, 1, "Pain Sprinkler"};
    g.blueprints[1].sheet = "thumbs_weapons.xml";
    g.blueprints[1].image = "thumb_pain_sprinkler";
    g.owned.insert(1);
    g.rafts.push_back({1, 1, 0, 0, 1000, 0, {{1, 2, 4, 1, 1000, 0, 0}, {2, 3, 1, 3, 1000, 0, 0}}});
    g.playerName = "Fixture player";
    g.materials[4] = 99999;
    g.materialNames[4] = "Gold";
    g.zones[1] = "Sailor's Sea";
    g.zoneMaps[1] = "xml_resources/maps/map_sailers_sea.tmx";
    g.mapX = 9;
    g.mapY = 21;
    g.daring = 1;
    g.buildLimit = 100;
    g.ready = true;
    g.status = "Fixture snapshot";
}
