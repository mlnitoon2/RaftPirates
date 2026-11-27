#include "rp/GridObjectHelpers.h"
#include <tuple>

namespace rp {
    namespace {
        bool named(const GridObject* o,const std::map<int,GridObjectInfo>& db,const char* name) {
            auto d=o?o->getResourceData(db):nullptr;
            return d&&d->name==name;
        }
    }
    bool IsRaft(const GridObject* o,const std::map<int,GridObjectInfo>& db) {
        return o&&o->getType(db)==0;
    }
    bool IsComponent(const GridObject* o,const std::map<int,GridObjectInfo>& db) {
        return o&&o->getType(db)==1;
    }
    bool IsWreckage(const GridObject* o,const std::map<int,GridObjectInfo>& db) {
        return o&&o->getType(db)==2;
    }
    bool IsBarracks(const GridObject* o,const std::map<int,GridObjectInfo>& db) {
        return named(o,db,"barracks");
    }
    bool IsDivingUnit(const GridObject* o,const std::map<int,GridObjectInfo>& db) {
        return named(o,db,"DivingUnit");
    }
    bool IsBossWeapon(const GridObject* o,const std::map<int,GridObjectInfo>& db) {
        for(const char* name:{"boss","boss01","boss02","boss03","boss04","boss05"})
            if(named(o,db,name)) return true;
        return false;
    }
    bool SortGridObjPredicate(const GridObject& a,const GridObject& b) {
        const auto x=a.GetGridPosition(),y=b.GetGridPosition();
        return std::tie(x.x,x.y)<std::tie(y.x,y.y);
    }
    std::string Tokenize(std::string& source,std::string delimiter) {
        // ARM32 consumes one byte after the found delimiter; single-byte separators
        // are the supported data format.
        const auto pos=delimiter.empty()?std::string::npos:source.find_first_of(delimiter);
        if(pos==std::string::npos){auto out=std::move(source);source.clear();return out;}
        auto out=source.substr(0,pos);
        source.erase(0,pos+1);
        return out;
    }
}
