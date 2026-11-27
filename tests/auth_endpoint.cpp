#include "../src/desktop/AuthEndpoint.h"
#include <stdexcept>
int main(){
    using rp::desktop::privateHttpEndpoint;
    for(auto url:{"http://192.168.1.207:900/auth.php","http://10.0.0.1:900/auth.php",
        "http://172.16.0.1/auth.php","http://172.31.255.255/auth.php","http://127.0.0.1/auth.php",
        "http://localhost:900/auth.php","http://[::1]:900/auth.php"})
        if(!privateHttpEndpoint(url))throw std::runtime_error("LAN endpoint rejected");
    for(auto url:{"https://192.168.1.207:900/auth.php","http://192.168.1.207.example.com/auth.php",
        "http://192.168.1.207@evil.com/auth.php","http://172.32.0.1/auth.php","http://8.8.8.8/auth.php",
        "http://192.168.001.1/auth.php","http://192.168.1.256/auth.php"})
        if(privateHttpEndpoint(url))throw std::runtime_error("Non-LAN endpoint accepted");
}
