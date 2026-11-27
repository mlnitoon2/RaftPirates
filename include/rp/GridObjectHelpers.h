#pragma once
#include "rp/Player.h"

namespace rp {
    // The portable GridObject stores an ID, not a native GridObjectData reference.
    // Accept the catalogue explicitly to preserve the legacy type/name tests.
    bool IsRaft(const GridObject*, const std::map<int, GridObjectInfo>&);
    bool IsComponent(const GridObject*, const std::map<int, GridObjectInfo>&);
    bool IsWreckage(const GridObject*, const std::map<int, GridObjectInfo>&);
    bool IsBarracks(const GridObject*, const std::map<int, GridObjectInfo>&);
    bool IsBossWeapon(const GridObject*, const std::map<int, GridObjectInfo>&);
    bool IsDivingUnit(const GridObject*, const std::map<int, GridObjectInfo>&);
    bool SortGridObjPredicate(const GridObject& a, const GridObject& b);
    std::string Tokenize(std::string& source, std::string delimiter);

    struct DoStuffExtraData {
        DoStuffExtraData();
        bool visited = false;
        int order = -1;
        GridObject::GridShapePiece* piece = nullptr;
        std::vector<GridObject::GridShapePiece*> predecessors;
    };

    bool MyDataSortPredicate(const DoStuffExtraData* a, const DoStuffExtraData* b);
    void Dostuff_place(std::map<GridObject::GridShapePiece*, DoStuffExtraData>& graph,
                       GridObject::GridShapePiece* piece, int& order);
    void performaIsoSort(std::vector<GridObject::GridShapePiece>& pieces,
                         std::vector<GridObject::GridShapePiece*>& sorted);
} // namespace rp
