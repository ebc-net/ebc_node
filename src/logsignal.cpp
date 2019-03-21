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
    tid.printNodeId();
    eng->startSearch(tid);
}

#endif
