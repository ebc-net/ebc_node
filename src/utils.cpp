#include <iostream>
#include "utils.h"
#include "sockaddr.h"
#include<QsLog.h>
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
       QLOG_ERROR()<<"resolve host error: ";

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
            QLOG_INFO()<<"[ "<<hbuf<<" ]";
        else
            QLOG_INFO()<<hbuf;
        if (std::strcmp(sbuf, "0"))
            QLOG_INFO()<< ":" << sbuf;
        else
            QLOG_INFO();
    } else
        QLOG_INFO()<< "[invalid address]";
}

void print_addr(const SockAddr &sa)
{
    print_addr(sa.get());
}
}
