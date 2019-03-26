#include "search.h"
#include "bucket.h"
#include <algorithm>
#include "QsLog.h"
#include<random>
#include<ctime>

namespace NET
{
Search::Searches::iterator Search::findSearchList(const NodeId &tid)
{
    decltype (searches.begin()) it ,itd = searches.end();
    for(it = searches.begin();it != searches.end();it ++)
    {
        if(it->tid == tid )
        {
            return it;
        }
        if(it->done && (itd == searches.end()))
        {
            itd = it;
        }
    }
    return itd;
}


bool Search::addSearchNode(Sp<Node> &node, NodeId tid)
{
    //找tid相应的search列表
    auto s = findSearchList(tid);
    if(s == searches.end())//如果没有tid的search,就建一个
    {
        if(searches.size()>1024)
            return false;
        searches.push_back(search(tid));//此处单设一个函数
        s = ++searches.rbegin().base();//?
    }
    auto id = s->tid;
    searchNode searchnode;
    searchnode.node = node;
//    if(s->searchNodes.size() == 0)//bug
//    {
//        s->searchNodes.push_back(searchnode);
//        return true;
//    }
    for (auto &n :s->searchNodes) //如果要插入的节点已经在列表中,则返回,此处是否与58冲突?
    {
        if(n.node == node)
            return true;
    }
    auto here = std::find_if(s->searchNodes.begin(),s->searchNodes.end(),
                             [&node,&id](searchNode &n)
    {
        return id.xorCmp(node->getId(),n.node->getId()) <= 0;
    });
    s->searchNodes.insert(here,searchnode);//insert searchnode
    if(s->searchNodes.size()>14)
        s->searchNodes.pop_back();
    return true;
}

//返回0：成功
//返回1：正在找
//返回－1：查找列表已满
//返回－2：失败
int Search::dhtSearch(NodeId tid, std::function<void (NodeId, Node &snode)> callback,Bucket::sendNode _send,bool isValue)
{

    if(kad->bucketIsEmpty(tid))//本地桶为空，直接返回
    {
        QLOG_WARN()<<"Search failed and bucket is empty";
        return -2;//空桶无法查找
    }
    send = _send;
    auto n = kad->getNode(tid);//先查找本地是否有此节点,如果有,则判断是否失效,如果找到好节点就返回成功
    if( n.get() != nullptr)
    {
        if(!n->isExpired())
        {
            callback(tid,*(n.get()));
            return 0;//成功返回0
        }
    }

    auto it = findSearchList(tid);//本地没找到,就去searchlist找
    if(it != searches.end())
    {
        if(!it->done)  //searching in progress
        {
            searchStep(it, send);
            it->step_time = clock::now();
            return 1;//think about
        }
        // new search
        it->done = 0;
        it->step_time = clock::now();
        it->tid =tid;

        for(auto iter = it->searchNodes.begin();iter!= it->searchNodes.end();)
        {
            if(iter->node->isExpired())
            {
                auto tempIt = iter++;
                it->searchNodes.erase(tempIt);//here need test!
                continue;
            }
            iter->replied= false;
            iter->requestTime = time_point ::min();
            iter++;
        }
    }

    auto nodes = kad->findClosestNodes(tid,8);
    if(nodes.size() == 0 )//找不到最近的节点,一个都找不到则返回失败
    {
        QLOG_WARN()<<"Search failed and has no nodes to send";
        return  -2;
    }
    QLOG_WARN()<<"find closest node : ";
    for(auto &n :nodes)
    {
        if(!addSearchNode(n,tid))
            return -1;
        n->getId().printNodeId();
    }
    searchStep(tid,send,3);//m=3
    return 0;
}
void Search::searchStep(NodeId &tid,Bucket::sendNode send,int m)
{
    auto it = findSearchList(tid);
    searchStep(it,send,m);
}
void Search::searchStep(const Searches::iterator &sr,Bucket::sendNode send,int m)
{
    bool isDone=true;
    int j = 0;
    /* Check if the first 8 live nodes have replied. */
    for(auto &n : sr->searchNodes)
    {
        if(j++>=8)
            break;
        if(n.node->isExpired())
            continue;
        if(!n.replied)
        {
            isDone = false;
            break;
        }
    }
    if(isDone)
    {
        sr->step_time = clock::now();
        sr->done = true;
        QLOG_INFO()<<"search is done ,but not found!";
        return;
    }
    //未达到发送时间（间隔10S）
    if(sr->step_time + seconds(10) >= clock::now())
        return;
    j = 0;
    for(auto &n : sr->searchNodes)
    {
        if(!n.replied && (n.requestTime < clock::now() - seconds(10)))
        {
            send(n.node,sr->tid);
            n.requestTime = clock::now();
            if(j++>=m)//给前3个发
                break;
        }
    }
    sr->step_time = clock::now();
    sr->done = false;
    return;
}
}
