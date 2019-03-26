#ifndef UTILS_H
#define UTILS_H


#include <memory>
#include <array>
#include <chrono>

namespace  NET
{
#define NET_VERSION    ("V0.0.0")

#define SUPER_NODE ("111.230.194.219")
#define NODE_PORT  (9443)
#define SRV_PORT   (9444)
#define ID_LENGTH  (20)
#define MAX_NODE   (8)
#define MAX_SEARCH_NODE  (14)
#define comPortNat(port,nat)  (((port)<<(16)) | ((nat)&(0xf)<<(12)))
#define parPort(value)        (((value)&(0xffff0000))>>16)
#define parNat(value)         (((value)&(0xf000)) >> 12)


template<class T>
using Sp = std::shared_ptr<T>;
using clock = std::chrono::steady_clock;
using time_point = clock::time_point;
using seconds = std::chrono::seconds;

}

#endif // UTILS_H
