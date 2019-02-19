/*************************************************************************
    > File Name: logLinux.cpp
    > Author: hxe
    > Mail: nomail 
    > Created Time: 2019年02月19日 星期二 11时47分09秒
 ************************************************************************/

#ifndef ON_QT
#include "QsLog.h"


static Cout *myout;

Cout::Cout()
{
    logFile.open("log.txt", std::ios::app|std::ios::out);
    if(logFile.is_open() == false)
    {
        std::cout<<"log file open fail"<<std::endl;
    }
}

Cout::~Cout()
{
    logFile.flush();
    logFile.close();
}

void Cout::writeFile(std::string log)
{
    std::lock_guard<std::mutex> lck(flock);
    logFile<<log;

    logFile.flush(); //flush every time is a bug
}

Cout* Cout::instance()
{
    if(!myout)
        myout = new Cout;

    return myout;
}

#endif
