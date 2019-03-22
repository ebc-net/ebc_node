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
    if(s == searches.end())
    {
        if(searches.size()>1024)
            return false;
        searches.push_back(search(tid));//此处单设一个函数
        s = ++searches.rbegin().base();
    }
    auto id = s->tid;
    auto here = std::find_if(s->searchNodes.begin(),s->searchNodes.end(),
                             [&node,&id](searchNode &n)
    {
        return id.xorCmp(node->getId(),n.node->getId()) <= 0;
    });
    searchNode searchnode;
    searchnode.node = node;
    s->searchNodes.insert(here,searchnode);//insert searchnode
    if(s->searchNodes.size()>14)
        s->searchNodes.pop_back();
    return true;
}

//返回0：成功
//返回1：正在找
//返回－1：查找列表已满
int Search::dhtSearch(NodeId tid, std::function<void (NodeId, Node &snode)> callback,Bucket::sendNode _send,bool isValue)
{
    send = _send;
    auto n = kad->getNode(tid);
    if( n.get() != nullptr)
    {
        if(!n->isExpired())
        {
            callback(tid,*(n.get()));
            return 0;//成功返回0
        }
    };

    auto it = findSearchList(tid);
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
	QLOG_WARN()<<"find closest node : ";
    for(auto &n :nodes)
    {
        if(!addSearchNode(n,tid))
            return -1;
		n->getId().printNodeId();
    }
    searchStep(tid,send);
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
            if(j++>=m)
                break;
        }
    }
    sr->step_time = clock::now();
    sr->done = false;
    return;
}
}
