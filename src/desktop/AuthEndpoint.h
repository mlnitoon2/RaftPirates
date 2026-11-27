#pragma once
#include <string>
#include <string_view>
#include <array>

namespace rp::desktop {
    // HTTP is allowed for an explicitly configured private IPv4 or loopback
    // server. No automatic TLS downgrade or DNS-based trust.
    inline bool privateHttpEndpoint(std::string_view endpoint) {
        if(endpoint.substr(0,7)!="http://")return false;
        auto authority=endpoint.substr(7);
        authority=authority.substr(0,authority.find('/'));
        if(authority.empty()||authority.find_first_of("@?#\\ \r\n\t")!=std::string_view::npos)return false;
        if(authority=="[::1]"||authority.substr(0,6)=="[::1]:")return true;
        auto host=authority.substr(0,authority.find(':'));
        if(host=="localhost")return true;
        std::array<unsigned,4> octets{};
        for(unsigned i=0;i<4;++i){
            auto dot=host.find('.');
            auto value=host.substr(0,dot);
            if(value.empty()||value.size()>3||(value.size()>1&&value.front()=='0'))return false;
            for(char c:value){if(c<'0'||c>'9')return false;octets[i]=octets[i]*10+unsigned(c-'0');}
            if(octets[i]>255)return false;
            if(i==3){if(dot!=std::string_view::npos)return false;}
            else {if(dot==std::string_view::npos)return false;host.remove_prefix(dot+1);}
        }
        return octets[0]==127||octets[0]==10||
            (octets[0]==192&&octets[1]==168)||
            (octets[0]==172&&octets[1]>=16&&octets[1]<=31);
    }
}
