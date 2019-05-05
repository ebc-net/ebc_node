#include "search.h"
#include "bucket.h"
#include <algorithm>
#include<random>
#include<ctime>

namespace NET
{
Search::Searches::iterator Search::findSearchList(const NodeId &tid)
{
    decltype (searches.begin()) it ,itd = searches.end();
    //QLOG_WARN()<<"searchlist size:"<<searches.size();
    for(it = searches.begin() ;it != searches.end();it ++)
    {
        if(it->tid == tid )
        {
            return it;
        }
        if((it->done ||it->findover)&& (itd == searches.end()))
        {
            QLOG_WARN()<<"test findsearchlist 2";
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
    searchnode.replied = false;
    searchnode.requiredTimes = 0;
    searchnode.requestTime = time_point::min();

    for (auto &n :s->searchNodes) //如果要插入的节点已经在列表中,则返回,此处是否与58冲突?
    {
        if(n.node == node)
        {
            //QLOG_INFO()<<"add the same node";
            //QLOG_WARN()<<"check the replied"<<n.replied;
            //node->getId().printNodeId();
            return true;
        }
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

//返回1：成功
//返回0：正在找
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
            return 1;//成功返回1
        }
    }

    auto it = findSearchList(tid);//本地没找到,就去searchlist找
    if(it != searches.end())
    {
        if(!it->done&&!it->findover)  //searching in progress
        {
            searchStep(it,callback, send);
            it->step_time = clock::now();

            return 0;//think about
        }
        // new search reset.
        it->done = 0;
        it->findover = 0;
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
            iter->requiredTimes = 0 ;
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
        n->getId().printNodeId(1);
        if(!addSearchNode(n,tid))
            return -1;
    }
    searchStep(tid,callback,send,3);//m=3
    if(it->done)
        return 1;
    if(it->findover)
        return -2;
    else
        return 0;

}
void Search::searchStep(NodeId &tid,std::function<void (NodeId, Node &snode)> callback,Bucket::sendNode send,int m)
{
    auto it = findSearchList(tid);
    searchStep(it,callback,send,m);
}
void Search::searchStep(const Searches::iterator &sr,std::function<void (NodeId, Node &snode)> callback,Bucket::sendNode send,int m)
{
    QLOG_WARN()<<"6 test done and findover"<< sr->done<<sr->findover;//TEST
    if(sr->done||sr->findover)
        return;
    bool isDone=true;
    int j = 0;
    /* Check if the first 8 live nodes have replied. */
    for(auto &n : sr->searchNodes)
    {
        if(j++>=8)
            break;
        if(n.node->isExpired() || n.requiredTimes >=2 )
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
        sr->findover = true;
        callback(sr->tid,*sr->searchNodes.begin()->node);
        //TEST
        QLOG_WARN()<<"print the searchnode list";
        for(auto &na : sr->searchNodes)
        {
            na.node->getId().printNodeId(1);
        }
        //QLOG_INFO()<<"search is done ,but not found!";
        return;
    }
    //未达到发送时间（间隔10S）
//    if(sr->step_time + seconds(10) >= clock::now())//这个时间后期加入网络环境再根据情况做调整
//        return;
    j = 0;
    for(auto &n : sr->searchNodes)
    {
        if(!n.replied && (n.requiredTimes <3)&&(n.requestTime < clock::now() - seconds(2)))
        {
            if(j++>=m)//给前3个发
                break;
            send(n.node,sr->tid);
            QLOG_ERROR()<<"send 3 node to search:"<<j;
            n.node->getId().printNodeId(1);
            n.requestTime = clock::now();
            n.requiredTimes += 1;

        }
    }
    sr->step_time = clock::now();
    return;
}
}
