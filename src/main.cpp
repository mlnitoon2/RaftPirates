#include "rp/PortView.h"
#include <iostream>
int main(int argc,char** argv){try{if(argc<3||argc>4){std::cerr<<"Usage: raftpirates_preview ASSET_ROOT OUTPUT.png [--map]\n";return 1;}rp::Game g;g.demo();rp::PortView v(argv[1]);v.grid=true;v.map=argc==4&&std::string(argv[3])=="--map";v.render(g,0,"OFFLINE PREVIEW").savePNG(argv[2]);std::cout<<g.summary()<<'\n';return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}}
