#ifndef NETENGINE_H
#define NETENGINE_H

#include <vector>
#include <map>
#include <thread>
//#include <threadpoolapiset.h>

#include "msgpack.h"
#include "bucket.h"
#include "node.h"
#include "utils.h"
#include"search.h"
#include "threadpool.h"
#ifdef ON_QT
#include <QNetworkConfigurationManager>
#endif

namespace NET
{
#ifdef ON_QT
class NetEngine: public QObject
#else
class NetEngine
#endif
{
#ifdef ON_QT
    Q_OBJECT
#endif
public:
    using sendNode = std::function<void(Sp<Node> &dstNode, NodeId tId)> ;
    NetEngine(const NodeId _id, Sp<Bucket>_kad, const bool _isServer = false);
    NetEngine() {}
    void NetInit(const NodeId _id, Sp<Bucket>_kad, const bool _isServer = false);
    void NetInit(const std::string createNetworkNodeAddress);
    ~NetEngine();
    void startServer();
    void startClient(const std::string ip = SUPER_NODE, const uint16_t port = SRV_PORT);
    bool sendUserData(NodeId tid, const char *data, const uint32_t sendDataStreamBufferSize);
    bool sendBroadcastData(NodeId tid, const char *data, const uint32_t sendDataStreamBufferSize);
    void printNodesInfo(int type = 0);   // 打印出所有节点信息
    void sendHello();
    bool startSearch(NodeId tId);
    int epollFd;  //

    Sp<Bucket> kad;//服务器或是客户机的K桶
    Node self;   //本节点的信息
    void setNodeExpired(const UDTSOCKET& sock/*, bool isServer = false*/);
    void eraseNodeExpired(const UDTSOCKET& sock/*, bool isServer = false*/);

    bool getUserData(std::string & data);
    bool joinNetwork(const std::string joinNetworkNodeAddress);
    bool getBucket(std::list<std::string> &_bucketList);
    bool eraseNode(const std::string breakNetworkNodeAddress);
    bool eraseNode(NodeId tId);
    bool sendDataStream(const std::string targetNodeAddress, const char *sendDataStreamBuffer, const uint32_t sendDataStreamBufferSize);
    int  getUserDataListSize();
    void brocastMsg(NodeId srcId, const char *sendDataStreamBuffer, const uint32_t sendDataStreamBufferSize);

private:
    void setUdtOpt(const UDTSOCKET &sock);   //设置socket的非阻塞以及发送/接收缓冲器的大小
    void handleMsg(UDTSOCKET,int epllFd = 0);
    int startPunch(int&, uint32_t ip, uint16_t port);
    //void appendBucket(const Sp<Node> &node);
    bool appendBucket(const Sp<Node> &node);
    void setSrvAddr(struct sockaddr_in cli,uint32_t aport);
    void bindSrv(struct sockaddr_in srv_addr,uint32_t port);

    Sp<Search> srch;
    UDTSOCKET boot_sock{0};  //本节点与服务器通信的SOCKET
    UDTSOCKET turn_sock{0}; //本节点与服务器中转的SOCKET
    UDTSOCKET srv{0};//srv socket
    std::thread boot_thread; //启动线程，负责与服务器节点交互以及维护对端节点的连接
    threadpool pool;
    bool boot_thread_flag;
    bool isServer{false};        //是否作为服务器
    std::thread  server;   //服务器线程(后期可能改为单独进程),当节点有成为服务器节点的可能时，启动此线程
    bool server_thread_flag;
    std::map<UDTSOCKET, Sp<Node>> sockNodePair;  //客户端
//    std::map<UDTSOCKET, Sp<Node>> sockNodePairSrv;  //服务器
    std::map<NodeId, NodeId> idTidPair; //存id和searchlist中tid的对应关系
    std::map<int,int> portsock;
    sockaddr_in local_addr ;
    time_point maintenanceTime;
    time_point searchTime;
    time_point expireTime;
    char *buf{nullptr};

    sendNode sendSearchNode;
    sendNode sendFindNode;
    std::function<void(NodeId tId, Node &sNode)> foundCallback;
    std::list<std::string> userData;
    std::list<Sp<Node>> blacklist;//黑名单，即被删除的节点，需要过滤
    int initCount{0};

    std::string srvip{SUPER_NODE};
    NodeId srvId{0};
    void turn( NodeId dstId,int ret);
#ifdef  ON_QT
    QNetworkConfigurationManager ebcNetConfig;
    bool isRelay{false};

#endif

};
}

#endif // NETENGINE_H
