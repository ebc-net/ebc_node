#ifndef NETENGINE_H
#define NETENGINE_H

#include <vector>
#include <map>
#include <thread>

#include "msgpack.h"
#include "bucket.h"
#include "node.h"
#include "utils.h"
#include"search.h"

namespace NET{
class NetEngine
{
public:

    NetEngine(const NodeId _id, Sp<Bucket>_kad,const bool _isServer = false);
    ~NetEngine();
    void startServer();
    void startClient(const std::string ip=SUPER_NODE, const uint16_t port=SRV_PORT);

    void printNodesInfo(int type = 0);   // 打印出所有节点信息
    void sendHello();
    void startSearch(NodeId tId);

private:
    void setUdtOpt(const UDTSOCKET &sock);   //设置socket的非阻塞以及发送/接收缓冲器的大小
    void handleMsg(UDTSOCKET, int epllFd = 0);
    int startPunch(int&,uint32_t ip, uint16_t port);
    //void appendBucket(const Sp<Node> &node);
    bool appendBucket(const Sp<Node> &node);
    void setNodeExpired(const UDTSOCKET& sock,bool isServer = false);

    Node self;   //本节点的信息
    Sp<Bucket> kad;//服务器或是客户机的K桶
    Sp<Search> srch;
    UDTSOCKET boot_sock;  //本节点与服务器通信的SOCKET
    std::thread boot_thread; //启动线程，负责与服务器节点交互以及维护对端节点的连接
    bool boot_thread_flag;
    bool isServer;        //是否作为服务器
    std::thread server;   //服务器线程(后期可能改为单独进程),当节点有成为服务器节点的可能时，启动此线程
    bool server_thread_flag;
    std::map<UDTSOCKET, Sp<Node>> sockNodePair;  //客户端
    std::map<UDTSOCKET, Sp<Node>> sockNodePairSrv;  //服务器
    std::map<NodeId,NodeId> idTidPair;//存id和searchlist中tid的对应关系
    sockaddr_in local_addr ;
    time_point maintenanceTime;
    time_point searchTime;
    time_point expireTime;
    char buf[6*1024]="";
    std::function<void(Sp<Node> &dstNode, NodeId tId)> sendSearchNode;
};
}

#endif // NETENGINE_H
