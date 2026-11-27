#pragma once
#include "sfs/SFSObjectWrapper.h"

namespace rp {
    class Zone {
      public:
        ~Zone();
        void initWithSFSObject(const sfs::SFSObjectWrapper& data);
        int id() const;
        const std::string& name() const;
        const std::string& mapfile() const;
        int min_bp() const;
        int max_bp() const;
        int buffer_bp() const;
        int daringLimit() const;
        int defaultpos_open_x() const;
        int defaultpos_open_y() const;
        int defaultpos_safe_x() const;
        int defaultpos_safe_y() const;

      private:
        int id_ = 0, min_ = 0, max_ = 0, buffer_ = 0, limit_ = 0;
        int openX_ = 0, openY_ = 0, safeX_ = 0, safeY_ = 0;
        std::string name_, map_;
    };
} // namespace rp
