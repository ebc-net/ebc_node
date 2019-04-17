#ifdef ON_QT
#include "logsignal.h"
#include "QsLog.h"
#include"bucket.h"
#include"netengine.h"
#include<QString>



void logSignal::start()
{
    kad->dump(0);
}

void logSignal::searchId(QString _id)
{
    NET::NodeId tid;
    std::string id = _id.toStdString();
    int ii = 0;

    for(int i = 0 ; i<ID_LENGTH*2-1; i+=2)
    {
        char high,low;

        if(id[i] >= '0' && id[i] <= '9')
            high = id[i] - '0';
        else if(id[i] >= 'a' && id[i] <= 'f')
            high = id[i] - 'a'+ 10;
        else
            return;

        if(id[i+1] >= '0' && id[i+1] <= '9')
            low = id[i+1] - '0';
        else if(id[i+1] >= 'a' && id[i+1] <= 'f')
            low = id[i+1] - 'a'+ 10;
        else
            return;

        tid[ii++]= high<<4|low;
    }
    QLOG_ERROR()<<"Search the node:";
    tid.printNodeId();
    eng->startSearch(tid);
}
void logSignal::sendData(QString _id, QString _data,QString _num)
{
    char* end;
    int num  = static_cast<int>(strtol(_num.toStdString().c_str(),&end,10));
    QLOG_WARN()<<"num = "<<num;
    NET::NodeId tid;
    const std::string id = _id.toStdString();
    //std::string data = _data.toStdString();
    std::string data;
    int ii = 0;

    for(int i = 0 ; i<ID_LENGTH*2-1; i+=2)
    {
        char high,low;

        if(id[i] >= '0' && id[i] <= '9')
            high = id[i] - '0';
        else if(id[i] >= 'a' && id[i] <= 'f')
            high = id[i] - 'a'+ 10;
        else
            return;

        if(id[i+1] >= '0' && id[i+1] <= '9')
            low = id[i+1] - '0';
        else if(id[i+1] >= 'a' && id[i+1] <= 'f')
            low = id[i+1] - 'a'+ 10;
        else
            return;

        tid[ii++]= high<<4|low;
    }

    char id1[27] = "ebc";
    memcpy(id1+3, tid.data(), ID_LENGTH);
    data.resize(num,'1');
    eng->sendDataStream(id1, data.c_str(), data.size());//size 问题
    QLOG_INFO()<<"send data stream to";
    tid.printNodeId();
}

void logSignal::printSelfId()
{
    QLOG_ERROR()<<"Self ID : ";
    eng->self.getId().printNodeId(1);
}

void logSignal::receiveData()
{
    QLOG_ERROR()<<"receive data:";

}


#endif
