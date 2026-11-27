#pragma once
#include "rp/Player.h"
namespace rp {
    // Catalogue-owned definitions are copied into resource snapshots.
    class GridObjectDataRes {
      public:
        explicit GridObjectDataRes(GridObjectInfo value);
        ~GridObjectDataRes();
        static std::shared_ptr<const GridObjectDataRes> Get(const std::string&,const std::map<int,GridObjectInfo>&);
        const GridObjectInfo& data()const{return data_;}
      private:
        GridObjectInfo data_;
    };
}
