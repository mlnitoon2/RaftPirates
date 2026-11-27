#include "rp/LoadingManager.h"
#include "rp/LoaderImage.h"
#include "rp/LoaderMenu.h"
#include "rp/LoaderManifest.h"
#include "sys/gfx/AssetImage.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace rp {
    LoadingManager::LoadingManager(std::filesystem::path root) : root_(std::move(root)) {}

    LoadingManager::~LoadingManager() = default;

    std::filesystem::path LoadingManager::checked(const std::string& name) const {
        std::filesystem::path path(name);
        if (name.empty() || name.find('\0') != std::string::npos ||
            name.find('\\') != std::string::npos || path.is_absolute() || path.has_root_name())
            throw std::invalid_argument("Invalid asset path");
        for (const auto& part : path)
            if (part == ".." || part == ".")
                throw std::invalid_argument("Invalid asset path");
        return root_ / path;
    }

    bool LoadingManager::resourceExists(const char* path, const char* extension) const {
        if (!path)
            return false;
        try {
            auto full = checked(path);
            if (extension && *extension && !full.has_extension())
                full.replace_extension(extension);
            return std::filesystem::is_regular_file(full);
        } catch (const std::exception&) {
            return false;
        }
    }

    void LoadingManager::enqueue(Kind kind, const std::string& path) {
        checked(path);
        if (jobs_.size() >= 10000)
            throw std::runtime_error("Manifest queue limit");
        if (seen_.emplace(kind, path).second)
            jobs_.push_back({kind, path});
    }

    void LoadingManager::addXml(const std::string& path) {
        enqueue(Kind::Xml, path);
    }

    void LoadingManager::addMenu(const std::string& path) {
        enqueue(Kind::Menu, path);
    }

    void LoadingManager::addSprite(const std::string& path) {
        enqueue(Kind::Sprite, path);
    }

    void LoadingManager::addAEAnim(const std::string& path) {
        enqueue(Kind::Animation, path);
    }

    void LoadingManager::addSheet(const std::string& path) {
        enqueue(Kind::Sheet, path);
    }

    void LoadingManager::addManifest(const std::string& path) {
        enqueue(Kind::Manifest, path);
    }

    void LoadingManager::prepareManifest(const std::string& path) {
        auto bytes = sys::File::readAll(checked(path));
        size_t pos = 0;
        auto u32 = [&]() {
            if (bytes.size() - pos < 4)
                throw std::runtime_error("Truncated resource manifest");
            uint32_t value = 0;
            for (unsigned i = 0; i < 4; ++i)
                value |= uint32_t(bytes[pos++]) << (8 * i);
            return value;
        };
        std::vector<Job> parsed;
        for (auto kind :
             {Kind::Sprite, Kind::Animation, Kind::Sheet, Kind::Menu, Kind::Manifest, Kind::Xml}) {
            uint32_t count = u32();
            if (count > 10000 || parsed.size() + count > 10000)
                throw std::runtime_error("Manifest entry limit");
            for (uint32_t i = 0; i < count; ++i) {
                uint32_t length = u32();
                size_t padded = (size_t(length) + 3) & ~size_t(3);
                if (!length || length > 4096 || padded > bytes.size() - pos ||
                    bytes[pos + length - 1] != 0)
                    throw std::runtime_error("Invalid manifest string");
                std::string name(reinterpret_cast<const char*>(bytes.data() + pos), length - 1);
                checked(name);
                pos += padded;
                parsed.push_back({kind, std::move(name)});
            }
        }
        if (pos != bytes.size())
            throw std::runtime_error("Unexpected manifest trailing data");
        auto nextSeen=seen_;
        auto nextJobs=jobs_;
        for(const auto& job:parsed)if(nextSeen.emplace(job.kind,job.path).second){
            if(nextJobs.size()>=10000)throw std::runtime_error("Manifest queue limit");
            nextJobs.push_back(job);
        }
        jobs_.swap(nextJobs);
        seen_.swap(nextSeen);
    }

    std::shared_ptr<sys::gfx::Image> LoadingManager::sprite(const std::string& path) {
        auto full = checked(path);
        if (!full.has_extension()) {
            auto png = full;
            png.replace_extension(".png");
            if (std::filesystem::is_regular_file(png))
                full = png;
            else
                full.replace_extension(".pvr");
        }
        return cache_.acquire<sys::gfx::Image>(full.string(),
                                               [&] { return sys::gfx::loadAssetImage(full); });
    }

    std::shared_ptr<sys::gfx::ResourceSpriteSheet> LoadingManager::sheet(const std::string& path) {
        auto full = checked(path);
        if (!full.has_extension())
            full.replace_extension(".xml");
        return cache_.acquire<sys::gfx::ResourceSpriteSheet>(
            full.string(), [&] { return sys::gfx::ResourceSpriteSheet::Create(full); });
    }

    void LoadingManager::loadImageJob(const std::string& path){
        if(path=="gfx/font/blue_highway"&&std::filesystem::is_regular_file(root_/"font/blue_highway.ttf"))
            cache_.acquire<std::vector<uint8_t>>(path,[&]{return sys::File::readAll(root_/"font/blue_highway.ttf");});
        else sprite(path);
    }
    void LoadingManager::loadMenuJob(const std::string& path){
        cache_.acquire<sys::res::XmlNode>(path,[&]{return sys::res::loadXML(checked(path));});
    }
    void LoadingManager::setPercentage(float value) {
        if (std::isfinite(value))
            progress_ = std::clamp(value, 0.f, 100.f);
    }

    void LoadingManager::start() {
        running_ = cursor_ < jobs_.size();
        setPercentage(jobs_.empty() ? 100.f : 100.f * cursor_ / jobs_.size());
    }

    void LoadingManager::cancel() {
        jobs_.clear();
        seen_.clear();
        errors_.clear();
        cursor_ = 0;
        running_ = false;
        setPercentage(0);
    }

    void LoadingManager::update(float dt) {
        if (!std::isfinite(dt) || dt < 0)
            return;
        cache_.update(dt);
        if (!running_)
            return;
        // At most one queued item per update to avoid blocking the whole startup frame.
        Job job = jobs_.at(cursor_++);
        try {
            switch (job.kind) {
            case Kind::Sprite:
                LoaderImage(job.path).load(*this);
                break;
            case Kind::Animation:
                cache_.acquire<sys::res::ResourceAEAnim>(
                    job.path, [&] { return sys::res::ResourceAEAnim::Create(checked(job.path)); });
                break;
            case Kind::Sheet:
                sheet(job.path);
                break;
            case Kind::Menu:
                LoaderMenu(job.path).prepare(this);
                break;
            case Kind::Xml:
                cache_.acquire<sys::res::XmlNode>(
                    job.path, [&] { return sys::res::loadXML(checked(job.path)); });
                break;
            case Kind::Manifest:
                LoaderManifest(job.path).prepare(this);
                break;
            }
        } catch (const std::exception& e) {
            errors_.push_back(job.path + ": " + e.what());
        }
        setPercentage(100.f * cursor_ / jobs_.size());
        running_ = cursor_ < jobs_.size();
    }
} // namespace rp
