#pragma once
#include "sys/res/Resources.h"
#include "sys/gfx/Gfx.h"
#include "sys/res/ResourceAEAnim.h"
#include <set>

namespace rp {
    class LoadingManager {
      public:
        explicit LoadingManager(std::filesystem::path root);
        ~LoadingManager();
        void addXml(const std::string& path);
        void addMenu(const std::string& path);
        void addSprite(const std::string& path);
        void addAEAnim(const std::string& path);
        void addSheet(const std::string& path);
        void addManifest(const std::string& path);
        bool resourceExists(const char* id, const char* type) const;
        bool assetExists(const char* path, const char* extension = "") const;
        void start();
        void cancel();
        void update(float dt);
        void setPercentage(float value);

        float percentage() const {
            return progress_;
        }

        bool running() const {
            return running_;
        }

        size_t completed() const {
            return cursor_;
        }

        size_t total() const {
            return jobs_.size();
        }

        const std::vector<std::string>& errors() const {
            return errors_;
        }

        std::shared_ptr<sys::gfx::Image> sprite(const std::string& path);
        std::shared_ptr<sys::gfx::ResourceSpriteSheet> sheet(const std::string& path);

      private:
        friend class LoaderAEAnim;
        friend class LoaderSpriteSheet;
        friend class LoaderXml;
        friend class LoaderImage;
        friend class LoaderMenu;
        friend class LoaderManifest;
        void loadImageJob(const std::string&);
        void loadMenuJob(const std::string&);
        enum class Kind { Sprite, Animation, Sheet, Menu, Manifest, Xml };

        struct Job {
            Kind kind;
            std::string path;
        };

        std::filesystem::path root_;
        sys::res::ResourceManager cache_;
        std::vector<Job> jobs_;
        std::set<std::pair<Kind, std::string>> seen_;
        std::vector<std::string> errors_;
        size_t cursor_ = 0;
        float progress_ = 0;
        bool running_ = false;
        void enqueue(Kind kind, const std::string& path);
        void prepareManifest(const std::string& path);
        std::filesystem::path checked(const std::string& path) const;
    };
} // namespace rp
