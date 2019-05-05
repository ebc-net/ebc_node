#ifndef SEARCH_H
#define SEARCH_H


#include <list>
#include <functional>
#include<bitset>

#include "bucket.h"
#include "nodeid.h"
#include "node.h"
#include "utils.h"
#include "QsLog.h"


namespace NET
{
struct searchNode
{
    Sp<Node> node;
    time_point requestTime;   //上次消息达到的时间,
    bool replied;
    int requiredTimes;
    searchNode():requestTime(time_point::min()),replied(false),requiredTimes(0){}
};

struct search
{
    NodeId tid;//充当tid和searchid
    time_point step_time;           /* the time of the last search_step */
    std::list<searchNode> searchNodes{};
    bool done;
    bool findover;//true 查找结束 未找到
    search(NodeId _tid):tid(_tid),done(false),findover(false){QLOG_WARN()<<"search init set done false";}
};
class Search
{
public:
    Search(Sp<Bucket> _kad):kad(_kad){searches.clear();}
    using Searches = std::list<search>;//searchlist
    Searches::iterator findSearchList(const NodeId& tid);
    bool addSearchNode(Sp<Node> &node, NodeId tid);
    void searchStep(const Searches::iterator &sr,std::function<void( NodeId tid,Node &sNode)> callback,Bucket::sendNode _send,int m =3 );//when startsearch m=3,when client call m=1;
    void searchStep(NodeId &tid,std::function<void (NodeId, Node &snode)> callback,Bucket::sendNode _send,int m =3 );//when startsearch m=3,when client call m=1;
    int dhtSearch(NodeId tid,std::function<void( NodeId tid,Node &sNode)> callback,Bucket::sendNode _send, bool isValue=false);
    Searches searches;
private:

    Sp<Bucket> kad;
    Bucket::sendNode send;
};

}
#endif // SEARCH_H
