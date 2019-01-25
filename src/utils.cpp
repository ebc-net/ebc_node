#include <iostream>
#include "utils.h"
#include "sockaddr.h"
#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#endif



namespace NET {

std::vector<SockAddr>
SockAddr::resolve(const std::string& host, const std::string& service)
{
    std::vector<SockAddr> ips {};
    if (host.empty())
        return ips;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    addrinfo* info = nullptr;
    int rc = getaddrinfo(host.c_str(), service.empty() ? nullptr : service.c_str(), &hints, &info);
    if(rc != 0)
        std::cout<<"resolve host error: "<<host<<std::endl;

    addrinfo* infop = info;
    while (infop) {
        ips.emplace_back(infop->ai_addr, infop->ai_addrlen);
        infop = infop->ai_next;
    }
    freeaddrinfo(info);
    return ips;
}

void print_addr(const sockaddr* sa, socklen_t slen)
{
    char hbuf[NI_MAXHOST];
    char sbuf[NI_MAXSERV];
    if (!getnameinfo(sa, slen, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV)) {
        if (sa->sa_family == AF_INET6)
            std::cout<<"[ "<<hbuf<<" ]";
        else
            std::cout<<hbuf;
        if (std::strcmp(sbuf, "0"))
            std::cout<< ":" << sbuf<<std::endl;
        else
            std::cout<<std::endl;
    } else
        std::cout<< "[invalid address]"<<std::endl;
}

void print_addr(const SockAddr &sa)
{
    print_addr(sa.get());
}
}
