#include "rp/Game.h"
#include "rp/SparseGrid.h"
#include "rp/MapView.h"
#include <stdexcept>
#include <iostream>
#define CHECK(x)                                                                                   \
    do {                                                                                           \
        if (!(x))                                                                                  \
            throw std::runtime_error(#x);                                                          \
    } while (false)

int main(int argc, char** argv) {
    try {
        rp::SparseGrid sparse;
        CHECK(sparse.addObject(1, {{0, 0}, {1, 0}}, {0, 0}));
        CHECK(sparse.addObject(2, {{0, 0}}, {3, 0}));
        CHECK(!sparse.moveObject(1, {2, 0}));
        CHECK(sparse.getObjectAt({0, 0}) == 1 && sparse.getObjectAt({3, 0}) == 2);
        CHECK(sparse.moveObject(1, {-2, -3}));
        CHECK(sparse.getObjectAt({-1, -3}) == 1 && !sparse.getObjectAt({0, 0}));
        CHECK(sparse.removeObject(1) && !sparse.hasObject(1));
        if (argc == 2) {
            rp::MapView map;
            map.setMapFile(argv[1], "xml_resources/maps/map_sailers_sea.tmx");
            CHECK(map.data());
        }
        std::cout << "Sparse grid and map checks passed\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
