#include "rp/PortView.h"
#include "sys/gfx/AssetImage.h"
#include <algorithm>
#include <cmath>
#include <cctype>
#include <sstream>

namespace rp {
    using namespace sys::gfx;
    using namespace sys::res;

    namespace {
        // Original implementation of a small debug-only 5x7 pixel alphabet.
        const char* glyph(char ch) {
            switch (std::toupper(static_cast<unsigned char>(ch))) {
            case 'A':
                return "0E11111F111111";
            case 'B':
                return "1E11111E11111E";
            case 'C':
                return "0F10101010100F";
            case 'D':
                return "1E11111111111E";
            case 'E':
                return "1F10101E10101F";
            case 'F':
                return "1F10101E101010";
            case 'G':
                return "0F10101711110F";
            case 'H':
                return "1111111F111111";
            case 'I':
                return "1F04040404041F";
            case 'J':
                return "0702020202120C";
            case 'K':
                return "11121418141211";
            case 'L':
                return "1010101010101F";
            case 'M':
                return "111B1515111111";
            case 'N':
                return "11191915131311";
            case 'O':
                return "0E11111111110E";
            case 'P':
                return "1E11111E101010";
            case 'Q':
                return "0E11111115120D";
            case 'R':
                return "1E11111E141211";
            case 'S':
                return "0F10100E01011E";
            case 'T':
                return "1F040404040404";
            case 'U':
                return "1111111111110E";
            case 'V':
                return "11111111110A04";
            case 'W':
                return "11111115151B11";
            case 'X':
                return "11110A040A1111";
            case 'Y':
                return "11110A04040404";
            case 'Z':
                return "1F01020408101F";
            case '0':
                return "0E11131519110E";
            case '1':
                return "040C040404040E";
            case '2':
                return "0E11010204081F";
            case '3':
                return "1E01010601011E";
            case '4':
                return "02060A121F0202";
            case '5':
                return "1F10101E01011E";
            case '6':
                return "0E10101E11110E";
            case '7':
                return "1F010204080808";
            case '8':
                return "0E11110E11110E";
            case '9':
                return "0E11110F01010E";
            case '-':
                return "0000001F000000";
            case ':':
                return "00040400040400";
            case '.':
                return "00000000000C0C";
            case '/':
                return "01010204081010";
            case ' ':
                return "00000000000000";
            default:
                return "0E110102040004";
            }
        }

        void rect(Image& i, int x, int y, int w, int h, Color c) {
            for (int yy = std::max(0, y); yy < std::min(i.height, y + h); ++yy)
                for (int xx = std::max(0, x); xx < std::min(i.width, x + w); ++xx)
                    i.at(xx, yy) = c;
        }

        int hex(char c) {
            return c >= 'A' ? c - 'A' + 10 : c - '0';
        }

        void text(Image& i, int x, int y, const std::string& s, int scale = 2) {
            for (char c : s) {
                if (x + 6 * scale >= i.width)
                    break;
                auto g = glyph(c);
                for (int row = 0; row < 7; ++row) {
                    int b = hex(g[row * 2]) * 16 + hex(g[row * 2 + 1]);
                    for (int col = 0; col < 5; ++col)
                        if (b & (1 << (4 - col)))
                            rect(i, x + col * scale, y + row * scale, scale, scale,
                                 {238, 245, 250, 255});
                }
                x += 6 * scale;
            }
        }

        void button(Image& i, int n, const std::string& s, bool active = false) {
            rect(i, n * 106 + 2, 350, 103, 46,
                 active ? Color{51, 115, 112, 255} : Color{29, 57, 74, 255});
            text(i, n * 106 + 8, 366, s, 1);
        }
    } // namespace

    PortView::PortView(std::filesystem::path root) : root_(std::move(root)), ocean_(root_) {}

    void PortView::loadMap(const Game& g) {
        auto i = g.zoneMaps.find(g.zone);
        if (i == g.zoneMaps.end() || i->second.empty()) {
            tmx_.reset();
            mapPath_.clear();
            return;
        }
        if (i->second == mapPath_ && tmx_)
            return;
        tmx_ = TmxMap::Load(root_ / i->second);
        mapPath_ = i->second;
    }

    Image PortView::render(const Game& g, float time, const std::string& status) {
        Image out(640, 400, {36, 133, 168, 255});
        if (!map)
            out = ocean_.render(g, time, grid);
        else {
            try {
                loadMap(g);
                if (tmx_) {
                    auto& m = *tmx_;
                    for (auto& l : m.layers)
                        for (int y = 0; y < l.height; ++y)
                            for (int x = 0; x < l.width; ++x) {
                                auto raw = l.at(x, y);
                                auto gid = raw & 0x1fffffffu;
                                if (!gid)
                                    continue;
                                const TmxTileset* set = nullptr;
                                for (auto& ts : m.tilesets)
                                    if (ts.firstGid <= gid && (!set || ts.firstGid > set->firstGid))
                                        set = &ts;
                                if (!set || set->tileWidth <= 0 || set->tileHeight <= 0)
                                    continue;
                                auto found = tiles_.find(set->image);
                                if (found == tiles_.end())
                                    found = tiles_
                                                .emplace(set->image,
                                                         std::make_shared<Image>(
                                                             loadAssetImage(root_ / set->image)))
                                                .first;
                                auto im = found->second;
                                int cols = im->width / set->tileWidth;
                                if (cols <= 0)
                                    continue;
                                auto local = gid - set->firstGid;
                                int sx = int(local % cols) * set->tileWidth,
                                    sy = int(local / cols) * set->tileHeight;
                                int dx = 48 + x * 544 / m.width, dy = 36 + y * 306 / m.height,
                                    dx1 = 48 + (x + 1) * 544 / m.width,
                                    dy1 = 36 + (y + 1) * 306 / m.height;
                                for (int yy = dy; yy < dy1; ++yy)
                                    for (int xx = dx; xx < dx1; ++xx) {
                                        int u = (xx - dx) * set->tileWidth / (dx1 - dx),
                                            v = (yy - dy) * set->tileHeight / (dy1 - dy);
                                        if (raw & 0x20000000u)
                                            std::swap(u, v);
                                        if (raw & 0x80000000u)
                                            u = set->tileWidth - 1 - u;
                                        if (raw & 0x40000000u)
                                            v = set->tileHeight - 1 - v;
                                        if (sx + u >= im->width || sy + v >= im->height)
                                            continue;
                                        auto p = im->at(sx + u, sy + v);
                                        auto& q = out.at(xx, yy);
                                        float a = p.a / 255.f;
                                        q = {uint8_t(p.r * a + q.r * (1 - a)),
                                             uint8_t(p.g * a + q.g * (1 - a)),
                                             uint8_t(p.b * a + q.b * (1 - a)), 255};
                                    }
                            }
                    int mx = 48 + (std::clamp(g.mapX, -100, 100) + 100) * 544 / 200,
                        my = 36 + (std::clamp(g.mapY, -100, 100) + 100) * 306 / 200;
                    rect(out, mx - 4, my - 4, 9, 9, {255, 222, 85, 255});
                }
            } catch (const std::exception&) {
                text(out, 50, 80, "MAP UNAVAILABLE");
            }
        }
        rect(out, 0, 0, 640, 32, {15, 38, 54, 255});
        text(out, 6, 4, "HYDRALEGACY RP PORT - " + status, 1);
        text(out, 6, 18,
             "ZONE " + std::to_string(g.zone) + " DARING " + std::to_string(g.daring) + " DIVE " +
                 std::to_string(g.diveSeconds) + "S",
             1);
        rect(out, 0, 342, 640, 58, {15, 38, 54, 255});
        button(out, 0, "CONNECT");
        button(out, 1, "GRID", grid);
        button(out, 2, map ? "OCEAN" : "MAP", map);
        button(out, 3, "DIVE");
        button(out, 4, "COLLECT");
        button(out, 5, "BLUEPRINT");
        if (grid && !map && !g.blueprints.empty()) {
            auto b = g.blueprints.begin();
            std::advance(b, selected % g.blueprints.size());
            rect(out, 0, 32, 640, 16, {15, 38, 54, 255});
            text(out, 8, 35, "PLACE: " + b->second.name + " - TAP GRID", 1);
        }
        return out;
    }

    std::optional<sfs::Message> PortView::touch(Game& g, float x, float y) {
        if (x < 0 || x >= 640 || y < 0 || y >= 400)
            return {};
        if (y >= 350) {
            int n = int(x) / 106;
            switch (n) {
            case 0:
                connectRequested = true;
                return {};
            case 1:
                grid = !grid;
                map = false;
                return {};
            case 2:
                map = !map;
                return {};
            case 3:
                return g.dive("dive_start");
            case 4:
                return g.dive("dive_collect");
            case 5:
                if (!g.blueprints.empty()) {
                    selected = (selected + 1) % g.blueprints.size();
                    auto it = g.blueprints.begin();
                    std::advance(it, selected);
                    if (!g.owned.count(it->first))
                        return g.buy(it->first);
                    grid = true;
                    map = false;
                }
                return {};
            default:
                return {};
            }
        }
        if (map) {
            loadMap(g);
            if (!tmx_ || x < 48 || x >= 592 || y < 36 || y >= 342)
                return {};
            float u = (x - 48) / 544.f, v = (y - 36) / 306.f;
            auto safe = tmx_->tileAtWorld("Safe Water", u * tmx_->width * tmx_->tileWidth,
                                          v * tmx_->height * tmx_->tileHeight);
            if (!safe)
                return {};
            return g.moveMap(int(u * 200 - 100), int(v * 200 - 100), *safe != 0);
        }
        if (grid && !g.blueprints.empty() && !g.rafts.empty()) {
            auto b = g.blueprints.begin();
            std::advance(b, selected % g.blueprints.size());
            auto& r = g.rafts.front();
            float bx = 320 + (r.x - r.y) * 192.f, by = 86 + (r.x + r.y) * 96.f;
            float dx = (x - bx) / 32.f, dy = (y - by) / 16.f;
            int cx = int(std::floor((dx + dy) / 2)), cy = int(std::floor((dy - dx) / 2));
            return g.build(b->second.object, r.id, cx, cy);
        }
        return {};
    }
} // namespace rp
