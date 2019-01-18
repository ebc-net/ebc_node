#include "netlog.h"

std::thread NET::netLog::logWriter;
std::mutex  NET::netLog::mtx;
std::condition_variable NET::netLog::cond;
std::queue<std::pair<NET::netLog::LOGLEVEL,std::string>> NET::netLog::logs;
bool NET::netLog::isInit;
std::streambuf* NET::netLog::back;
std::fstream NET::netLog::f;


int NET::netLog::logInit()
{
    if(isInit)
        return 0;

    isInit = true;
    return 0;
}



int NET::netLog::logInit(const std::string &filename)
{
    if(isInit)
        return 0;

    f.open(filename, std::ios::out| std::ios::app);
    if(f.is_open() == false)
    {
        std::cout<<"[ERROR] file "<<filename<<"open fail"<<std::endl;
        return -1;
    }

    back = std::cout.rdbuf();
    std::cout.rdbuf(f.rdbuf());

    logWriter = std::thread([this]()
    {
    }
    );

    return 0;
}
