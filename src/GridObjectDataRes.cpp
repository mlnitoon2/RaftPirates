#include "rp/GridObjectDataRes.h"
namespace rp {
    GridObjectDataRes::GridObjectDataRes(GridObjectInfo value):data_(std::move(value)){}
    GridObjectDataRes::~GridObjectDataRes()=default;
    std::shared_ptr<const GridObjectDataRes> GridObjectDataRes::Get(const std::string& name,const std::map<int,GridObjectInfo>& db){
        for(const auto& entry:db)if(entry.second.name==name)
            return std::make_shared<const GridObjectDataRes>(entry.second);
        return {};
    }
}
