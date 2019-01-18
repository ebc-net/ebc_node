#ifndef NETLOG_H
#define NETLOG_H

#include <iostream>
#include <fstream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>


namespace NET {

class netLog
{
    typedef  enum _LOGLEVEL
    {
        LOGINFO = 0,
        LOGWARN ,
        LOGERROR,
    }LOGLEVEL;

    static int logInit();
    static int logInit(const std::string& filename);

    static int logPrinf(const LOGLEVEL& level,  const char * formal,...);

private:
    static std::thread logWriter;
    static std::mutex  mtx;
    static std::condition_variable cond;
    static std::queue<std::pair<LOGLEVEL,std::string>> logs;
    static bool isInit;
    static std::streambuf *back;
    static std::fstream f;
};

}

#endif // NETLOG_H
