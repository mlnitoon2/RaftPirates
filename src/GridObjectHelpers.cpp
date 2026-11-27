#include "rp/GridObjectHelpers.h"
#include <tuple>
#include <algorithm>
#include <stdexcept>
#include <limits>

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
    DoStuffExtraData::DoStuffExtraData()=default;
    bool MyDataSortPredicate(const DoStuffExtraData* a,const DoStuffExtraData* b) {
        return a->order<b->order;
    }
    void Dostuff_place(std::map<GridObject::GridShapePiece*,DoStuffExtraData>& graph,
                       GridObject::GridShapePiece* piece,int& order) {
        // Mark visited before traversing dependencies as in ARM32 0x475208.
        // Use an iterative stack to avoid unbounded recursion for damaged data.
        using Piece=GridObject::GridShapePiece;
        if(!piece||graph.at(piece).visited)return;
        std::vector<std::pair<Piece*,size_t>> stack{{piece,0}};
        graph.at(piece).visited=true;
        while(!stack.empty()){
            auto& top=stack.back();
            auto& current=graph.at(top.first);
            if(top.second<current.predecessors.size()){
                auto* next=current.predecessors[top.second++];
                auto& dep=graph.at(next);
                if(!dep.visited){dep.visited=true;stack.emplace_back(next,0);}
            }else{
                if(order==std::numeric_limits<int>::max())throw std::overflow_error("Iso order overflow");
                current.order=order++;stack.pop_back();
            }
        }
    }
    void performaIsoSort(std::vector<GridObject::GridShapePiece>& pieces,
                         std::vector<GridObject::GridShapePiece*>& sorted) {
        sorted.clear();
        if(pieces.size()>4096)throw std::invalid_argument("Iso shape count limit");
        std::map<GridObject::GridShapePiece*,DoStuffExtraData> graph;
        for(auto& p:pieces)graph[&p].piece=&p;
        for(size_t i=0;i<pieces.size();++i)for(size_t j=i+1;j<pieces.size();++j){
            auto& a=pieces[i];auto& b=pieces[j];
            const bool ab=a.precedes(b),ba=b.precedes(a);
            if(ab&&!ba)graph[&b].predecessors.push_back(&a);
            if(ba&&!ab)graph[&a].predecessors.push_back(&b);
        }
        int order=0;
        for(auto& p:pieces)Dostuff_place(graph,&p,order);
        std::vector<DoStuffExtraData*> records;
        for(auto& p:pieces)records.push_back(&graph.at(&p));
        std::stable_sort(records.begin(),records.end(),MyDataSortPredicate);
        for(auto* p:records)sorted.push_back(p->piece);
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
