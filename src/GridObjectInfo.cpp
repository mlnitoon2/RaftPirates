#include "rp/Player.h"
#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace rp {
    namespace {
        int number(const std::string& s) {
            size_t used = 0;
            int n = std::stoi(s, &used);
            if (used != s.size())
                throw std::runtime_error("Invalid grid integer");
            return n;
        }
    } // namespace

    GridObjectPiece::GridObjectPiece(const std::string& data) {
        std::istringstream s(data);
        if (!(s >> asset >> offsetX >> offsetY))
            throw std::runtime_error("Invalid grid object piece");
        std::string xy;
        while (s >> xy) {
            auto comma = xy.find(',');
            if (comma == std::string::npos)
                throw std::runtime_error("Invalid grid cell");
            cells.push_back({number(xy.substr(0, comma)), number(xy.substr(comma + 1))});
        }
    }

    GridObjectPiece::~GridObjectPiece() = default;

    GridObjectInfo GridObjectInfo::parse(const std::string& data, int battlePoints) {
        GridObjectInfo info;
        info.bp = std::max(0, battlePoints);
        std::istringstream stream(data);
        std::string part;
        if (std::getline(stream, part, ';') && !part.empty())
            info.type = number(part);
        while (std::getline(stream, part, ';')) {
            if (part.empty())
                continue;
            GridObjectPiece piece(part);
            if (info.pieces.empty()) {
                info.asset = piece.asset;
                info.offsetX = piece.offsetX;
                info.offsetY = piece.offsetY;
            }
            for (auto c : piece.cells)
                if (std::none_of(info.cells.begin(), info.cells.end(),
                                 [&](auto a) { return a.x == c.x && a.y == c.y; }))
                    info.cells.push_back(c);
            info.pieces.push_back(std::move(piece));
        }
        if (info.cells.empty())
            info.cells.push_back({0, 0});
        return info;
    }
} // namespace rp
