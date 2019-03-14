#ifdef ON_QT
#include "logsignal.h"
#include "QsLog.h"
#include"bucket.h"




void logSignal::start()
{
    kad->dump();
}

void logSignal::searchId(std::string &id)
{
    NET::NodeId search_id(id);
    //加search结构？
    if(kad->findNode(id))
    {
        QLOG_INFO()<<"find the node in local buckets";
        return;
    }

    auto targetNodes = kad->findClosestNodes(id,3);
    //调用搜索函数向这三个node搜索此id




}

#endif
