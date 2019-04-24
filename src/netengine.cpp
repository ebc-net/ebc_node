#include "netengine.h"
#include "bucket.h"
#include "search.h"
#include <iostream>
#include <cstring>
#include "QsLog.h"
#include <chrono>//C++ time
#ifdef ON_QT
#include <QtGlobal>
#endif
namespace NET
{
NetEngine::NetEngine(const NodeId _id, Sp<Bucket>_kad, const bool _isServer): self(_id), kad(_kad), isServer(_isServer)
{
    NetInit(_id, _kad, _isServer);
}

void NetEngine::NetInit(const NodeId _id, Sp<Bucket> _kad, const bool _isServer)
{

    if(initCount++)
        return ;

    buf = new char[1024 * 101];
    kad = _kad;

    self = _id;
    self.getId().printNodeId();
    isServer = _isServer;
//    auto netconfigList = ebcNetConfig.allConfigurations(QNetworkConfiguration::Active);
//    for(auto netcon : netconfigList)
//        QLOG_INFO()<< netcon.bearerTypeName().toStdString().c_str();

#if 0
    using namespace QsLogging;

    // 1. init the logging mechanism
    Logger& logger = Logger::instance();
    logger.setLoggingLevel(QsLogging::TraceLevel);
    const QString sLogPath("log.txt");

    // 2. add two destinations
    DestinationPtr fileDestination(DestinationFactory::MakeFileDestination(
                                       sLogPath, EnableLogRotation, MaxSizeBytes(1), MaxOldLogCount(2)));
    DestinationPtr debugDestination(DestinationFactory::MakeDebugOutputDestination());
    //DestinationPtr functorDestination(DestinationFactory::MakeFunctorDestination(&logFunction));
    logger.addDestination(debugDestination);
    logger.addDestination(fileDestination);
    //logger.addDestination(functorDestination);
#endif

    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = 0;
    local_addr.sin_family = AF_INET;
    srch = std::make_shared<Search> (kad);
    std::list<std::string> onlineNodeAddress{};//jian yue

    sendSearchNode = [&](Sp<Node> &dstNode, NodeId tId)
    {
        char send_buf[1024] = "";
        config::search searchNode;
        searchNode.set_isid(true);
        searchNode.set_tid(&tId, ID_LENGTH);
        msgPack sendMsg(self.getId());
        int msg_len = sendMsg.pack(config::MsgType::GET_DATA, &searchNode, send_buf, sizeof(send_buf));//只传ID去
        if(msg_len < 0)
        {
            QLOG_ERROR() << "SendSearchNode msglen error";
            return ;
        }
        int ret = UDT::sendmsg(dstNode->getSock(), send_buf, msg_len);
//        QLOG_WARN() << "sendmsg to searchnode" << ret;
    };

    sendFindNode = [&](Sp<Node> &dstNode, NodeId targetId)
    {
        char fbuf[1024] = "";
        config::EbcNode targetNode;
        targetNode.set_id(targetId.toString());
        msgPack sendMsg(self.getId());
        int msg_len = sendMsg.pack(config::MsgType::GET_NODE, &targetNode, fbuf, sizeof(fbuf));//只传ID去
        if(msg_len < 0)
            return ;
        UDT::sendmsg(dstNode->getSock(), fbuf, msg_len);
    };

}

void NetEngine::NetInit(const std::string createNetworkNodeAddress)
{
    NodeId id{};
    id = createNetworkNodeAddress.substr(3, ID_LENGTH);
    Sp<NET::Bucket>  __kad = std::make_shared <NET::Bucket>(id);
    NetInit(id, __kad);
}

NetEngine::~NetEngine()
{
    boot_thread_flag = false;
    server_thread_flag = false;
    if(boot_sock)
        UDT::close(boot_sock);
    boot_sock = 0;
    if(kad.use_count())
    {
        kad->closeBucket([](int sock)
        {
            UDT::close(sock);
        });
    }
    delete []buf;
//    QsLogging::Logger::destroyInstance();
}

void NetEngine::startServer()
{
    expireTime = clock::now() + seconds(300);
    if(self.getId().empty())
    {
        QLOG_ERROR() << "Server net start fail: Id is empty";
        return ;
    }
    isServer = true;
    server_thread_flag = true;
    //server thread .if the node is srv,run this thread.
    server = std::thread( [&]()
    {
        UDTSOCKET srv = UDT::socket(AF_INET, SOCK_DGRAM, 0);
        UDTSOCKET cli;

        int epollFd = UDT::epoll_create();
        int event = 0 ;
        int ret = 0;
        std::set<UDTSOCKET> readfds;
        std::set<UDTSOCKET> errfds;
        event = UDT_EPOLL_IN | UDT_EPOLL_ERR; //Readable|Error

        int sock_len = sizeof(struct sockaddr);
        struct sockaddr_in srv_addr, client;
        srv_addr.sin_addr.s_addr = INADDR_ANY;
        srv_addr.sin_port = htons(SRV_PORT);
        srv_addr.sin_family = AF_INET;

        setUdtOpt(srv);
        if(UDT::bind(srv, (struct sockaddr*)&srv_addr, sizeof(srv_addr)) < 0)
        {
            QLOG_ERROR() << "UDT bind server error" << UDT::getlasterror().getErrorMessage();
            return ;
        }

        if(UDT::listen(srv, 10000) < 0)
        {
            QLOG_ERROR() << "UDT listen server error" << UDT::getlasterror().getErrorMessage();
            return ;
        }
        UDT::epoll_add_usock(epollFd, srv, &event);
        while(server_thread_flag)
        {
            //the wait only listen srv's socket and its return of client's socket
            ret = UDT::epoll_wait(epollFd, &readfds, nullptr, &errfds, 5 * 1000);

            if(ret < 0)
            {
                QLOG_ERROR() << "UDT epoll_wait error" << UDT::getlasterror().getErrorMessage();
                return ;
            }
            else if(ret > 0)
            {
                for(auto& sock : errfds) //Heartbeat fail!
                {
                    //Error
                    int state = 0;
                    int len = sizeof(state);
                    UDT::getsockopt(sock, 0, UDT_STATE, &state, &len);
                    //the srv has no state = CONNECTING ,as it should not connect.
                    if(CLOSED == state || BROKEN == state )
                    {
                        QLOG_INFO() << "client disconnected";
                        UDT::epoll_remove_usock(epollFd, sock);
                        UDT::close(sock);
                        setNodeExpired(sock, true);
                        eraseNodeExpired(sock, true);
                    }
                }
                for(auto& sock : readfds) //readable ,have msg come
                {
                    if(sock == srv) //client connects success!
                    {
                        if((cli = UDT::accept(srv, (struct sockaddr*)&client, &sock_len)) < 0)
                        {
                            QLOG_ERROR() << "UDT accept error" << UDT::getlasterror().getErrorMessage();
                            continue;
                        }

                        QLOG_INFO() << "peer info:" << inet_ntoa(client.sin_addr) << "@" << ntohs(client.sin_port);
                        QLOG_INFO() << "cli = " << cli;
                        //这里要不要设置cli这个socket的属性，还是说cli会继承srv套接字的属性
                        UDT::epoll_add_usock(epollFd, cli, &event);
                    }
                    else
                    {
                        //msg of client
                        int state = 0;
                        int len = sizeof(state);
                        UDT::getsockopt(sock, 0, UDT_STATE, &state, &len);//shut down
                        //the srv has no state = CONNECTING ,as it should not connect.
                        if(CLOSED == state || BROKEN == state )
                        {
                            QLOG_INFO() << "client disconnected";
                            UDT::epoll_remove_usock(epollFd, sock);
                            UDT::close(sock);
                            setNodeExpired(sock, true);
                            eraseNodeExpired(sock, true);
                            continue;
                        }
                        //recieve and handle msg
                        handleMsg(sock) ;
                    }
                }
            }

            if(clock::now() >= expireTime)
            {
                kad->expireBucket();//删掉节点
                expireTime = clock::now() + seconds(300);
            }
        }
    }
                        );
    server.detach();
}

void NetEngine::startClient(const std::string ip, const uint16_t port)//指定服务器的ip和端口号
{
    maintenanceTime = clock::now();
    searchTime = clock::now();
    boot_sock = UDT::socket(AF_INET, SOCK_DGRAM, 0);
    if(boot_sock < 0)
    {
        QLOG_ERROR() << "UDT socket error" << UDT::getlasterror().getErrorMessage();
        return ;
    }

    struct sockaddr_in srv_addr;
    srv_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    srv_addr.sin_port = htons(port);
    srv_addr.sin_family = AF_INET;

    setUdtOpt(boot_sock);
    if(UDT::bind(boot_sock, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0)
    {
        QLOG_ERROR() << "UDT bind error" << UDT::getlasterror().getErrorMessage();
        return ;
    }

    int sock_len = sizeof(local_addr);
    if(UDT::getsockname(boot_sock, (struct sockaddr *)&local_addr, &sock_len) < 0)//获取本地地址，正确返回0。
    {
        QLOG_ERROR() << "UDT getsockname error" << UDT::getlasterror().getErrorMessage();
        return ;
    }
    QLOG_INFO() << "local port = " << ntohs(local_addr.sin_port);

    boot_thread_flag = true;
    boot_thread = std::thread([&, srv_addr]()  //lambda 表达式
                              //与服务器交互的线程
    {
        if(UDT::connect(boot_sock, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) < 0)
        {
            QLOG_ERROR() << "UDT boot socket connect error" << UDT::getlasterror().getErrorMessage();
            return ;
        }

        QLOG_INFO() << "srv info" << inet_ntoa(srv_addr.sin_addr) << "@" << htons(srv_addr.sin_port);
        char cbuf[1024] = "";
        epollFd = UDT::epoll_create();
        int event = 0 ;
        int ret = 0;
        int state = 0;
        int len = sizeof(state);

        std::set<UDTSOCKET> readfds, writefds, errfds;

        /*监听套接字的可写，错误*/
        event = UDT_EPOLL_OUT | UDT_EPOLL_ERR;
        UDT::epoll_add_usock(epollFd, boot_sock, &event);

        while(boot_thread_flag)
        {
            ret = UDT::epoll_wait(epollFd, &readfds, &writefds, &errfds, 1 * 1000);

            //程序关闭时，直接退出
            if(!boot_thread_flag)
                return ;
            if(ret < -1)
            {
                QLOG_ERROR() << "UDT epoll_wait error" << UDT::getlasterror().getErrorMessage();
                return ;
            }
            else if(ret > 0)
            {

                for(auto& sock : errfds)
                {
                    // UDT::getsockopt(sock, 0, UDT_STATE, &state, &len);
                    //QLOG_INFO()<<"in errfds state = "<<state;
                    UDT::epoll_remove_usock(epollFd, sock);
                    if(sock == boot_sock)
                    {
                        //如果是服务器连接失败，则做重连动作  TODO
                        //UDT有问题，不能连续连接 注意一下
                        QLOG_ERROR() << "server connect error! re-connecting...";
                        //此处封装一层，SOCK关掉重连
                        //if(UDT::connect(boot_sock, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) < 0)
                        //{
                        //	QLOG_ERROR()<<"UDT boot socket connect error"<<UDT::getlasterror().getErrorMessage();
                        //	return ;
                        //}
                        //event = UDT_EPOLL_OUT|UDT_EPOLL_ERR;
                        //UDT::epoll_add_usock(epollFd, sock, &event);
                    }
                    else
                    {
                        QLOG_ERROR() << "peer connect error1 sock=" << sock;
                        sockNodePair[sock]->getId().printNodeId(true);
                        UDT::close(sock);
                        setNodeExpired(sock);
                        eraseNodeExpired(sock);
                    }
                }
                /*可写，这里表示connect连接状态, 在套接字可写后，获取套接字的状态，根据不同的状态可以知道socket的连接状态*/
                for(auto& sock : writefds)
                {
                    event = UDT_EPOLL_IN | UDT_EPOLL_ERR;
                    UDT::epoll_remove_usock(epollFd, sock);
                    UDT::epoll_add_usock(epollFd, sock, &event);

                    if(sock == boot_sock) //服务器连接成功
                    {
                        //发送GET_NODE命令到服务器
                        config::EbcNode self_node;
                        self_node.set_port_nat(comPortNat(0, self.getNatType()));
                        msgPack sendMsg(self.getId());
                        int msg_len = sendMsg.pack(config::MsgType::GET_NODE, &self_node, cbuf, sizeof(cbuf));
                        if(msg_len < 0)
                            continue;
                        UDT::sendmsg(sock, cbuf, msg_len);
                    }
                    else
                    {
                        //连接对端节点成功，则将改节点的socket记录,将此节点加入K桶
                        QLOG_INFO() << "peer node connect success!";
                        Sp<Node>& node = sockNodePair[sock];//记录UDT套接字与节点ID的对应关系，方便通过UDTSOCKET快速找到ID
                        auto tid = idTidPair.find(node->getId());
                        if((tid != idTidPair.end())/*&&(!srch->findSearchList(tid->second)->done)*/)
                        {
                            srch->addSearchNode(node, tid->second);
                            srch->searchStep(tid->second, sendSearchNode, 1);
                        }
                        if(!appendBucket(node))//if appendBucket failed,close the sock and remove it.
                        {
                            UDT::epoll_remove_usock(epollFd, sock);
                            UDT::close(sock);
                            sockNodePair.erase(sock);
                            QLOG_ERROR() << "ADD BUCKET FAILED";
                        }
                    }
                }
                /*有消息达到*/
                for(auto& sock : readfds)
                {
                    UDT::getsockopt(sock, 0, UDT_STATE, &state, &len);
                    /*这个时候的失败一般是之前连接上了，但是中间连接断开了*/
                    if(CLOSED == state || BROKEN == state || CONNECTING == state)
                    {
                        if(sock == boot_sock)
                        {
                            QLOG_ERROR() << "Srv is down!"; //服务器下线~
                            UDT::epoll_remove_usock(epollFd, sock);
                            continue;

                        }
                        UDT::epoll_remove_usock(epollFd, sock);
                        QLOG_ERROR() << "client connect error, ID is :";
                        sockNodePair[sock]->getId().printNodeId(true);
                        UDT::close(sock);
                        //失效
                        setNodeExpired(sock);
                        eraseNodeExpired(sock);                     
                    }
                    handleMsg(sock, epollFd) ;
                }
            }

            //search time maintenance

            if(searchTime <= clock::now())
            {
                for (auto it = srch->searches.begin(); it != srch->searches.end(); it++)
                {
                    if(!it->done && ((it->step_time + seconds(2)) <= clock::now()))//6s before
                        srch->searchStep(it, sendSearchNode, 1);
                    time_point tm ;
                    if(!it->done)
                        tm = it->step_time + seconds(5);
                    if(tm < searchTime)
                        searchTime = tm;
                }
            }

            if(clock::now() >= maintenanceTime)
            {
                if( kad->bucketMaintenance( sendFindNode, false )) //neighbour=false 桶维护
                {
                    maintenanceTime = clock::now() + seconds(5);
                    QLOG_INFO() << "send get_node for bucket maintenance";
                }
                else
                {
                    maintenanceTime = clock::now() + seconds(60);

                    if(kad->grow_time <= clock::now() - seconds(150))//mybucket加入新节点后150s再扩桶
                    {
                        kad->bucketMaintenance(sendFindNode, true); //扩
                        QLOG_INFO() << "send get_node for self maintenance";
                        kad->grow_time = clock::now();
                    }
                }
            }
        }
    });
    boot_thread.detach();
}

bool NetEngine::sendUserData(NodeId tid, const char *data, const uint32_t sendDataStreamBufferSize)
{
    int sock = 0;
    std::vector<char> sbuf;
    sbuf.resize(1024 * 1024);
    Sp<Node> node = kad->getNode(tid);
    if(node.get() == nullptr)
        return false;

    sock = node->getSock();
    msgPack send_msg(self.getId());
    int ret = send_msg.pack(config::MsgType::SENDDATASTREAM, data, sbuf.data(), sbuf.size(), config::MsgSubType::EMPTY_SUB, sendDataStreamBufferSize);
    if(ret < 0)
        return false;
    ret = UDT::sendmsg(sock, sbuf.data(), ret);
    if(ret < 0)
        return false;
    return true;
}

bool NetEngine::startSearch(NodeId tId)//传tid进来
{
    foundCallback = [&](NodeId tId, Node & sNode)
    {
        QLOG_WARN() << "Find the search Node :";
        tId.printNodeId();
        //        QLOG_WARN()<<"Find in the Node:";
        //        sNode.getId().printNodeId();
    };
    auto herenode = std::find_if(blacklist.begin(), blacklist.end(),
                             [&tId](std::list<Sp<Node>>::value_type & blacknode)
    {
        return blacknode->getId() == tId;
    });
    if(herenode != blacklist.end())
    {
       QLOG_WARN()<<"erase the blacknode :";
       herenode->get()->getId().printNodeId(1);
       blacklist.erase(herenode);
    }
    if(tId == self.getId())
    {
        return true;
        QLOG_WARN() << "Find the search Node :It is myself Id";
    }
    else
        return (srch->dhtSearch(tId, foundCallback, sendSearchNode) == 1) ? true : false ;
}

void NetEngine::setUdtOpt(const UDTSOCKET &sock)//srv
{
    bool opt = false;
    if(UDT::setsockopt(sock, 0, UDT_SNDSYN, &opt, sizeof(opt)) < 0)//etsockopt对套接字进行设置// if sending is blocking
    {
        QLOG_ERROR() << "UDT set SNDSYN fail" << UDT::getlasterror().getErrorMessage();
        return ;
    }

    if(UDT::setsockopt(sock, 0, UDT_RCVSYN, &opt, sizeof(opt)) < 0)
    {
        QLOG_ERROR() << "UDT set RCVSYN fail" << UDT::getlasterror().getErrorMessage();
        return ;
    }

    int size = 2 * 1024 * 1024;
    if(UDT::setsockopt(sock, 0, UDT_SNDBUF, &size, sizeof(size)) < 0)
    {
        QLOG_ERROR() << "UDT set SNDBUF fail" << UDT::getlasterror().getErrorMessage();
        return ;
    }

    if(UDT::setsockopt(sock, 0, UDT_RCVBUF, &size, sizeof(size)) < 0)
    {
        QLOG_ERROR() << "UDT set RCVBUF fail" << UDT::getlasterror().getErrorMessage();
        return ;
    }

#ifdef Q_OS_MAC

    int s_size = sizeof(int);
    if(UDT::getsockopt(sock, 0, UDP_RCVBUF, &size, &s_size) < 0)
    {
        QLOG_ERROR() << "UDT set RCVBUF fail" << UDT::getlasterror().getErrorMessage();
        return ;
    }
    QLOG_INFO() << "udp buf size = " << size;

    size = 2 * 1024 * 1024;
    if(UDT::setsockopt(sock, 0, UDP_RCVBUF, &size, sizeof(size)) < 0)
    {
        QLOG_ERROR() << "UDT set RCVBUF fail" << UDT::getlasterror().getErrorMessage();
        return ;
    }
#endif
}

void NetEngine::handleMsg(UDTSOCKET sock, int epollFd)//handleMsg(sock）
{
    memset(buf, 0, BUFSIZE);
    msgPack recv_msg(self.getId());
    int ret = UDT::recvmsg(sock, buf, BUFSIZE);
    //QLOG_WARN()<<"handlemsg recv ret = "<<ret;
    if(ret < 0)
    {
//        QLOG_ERROR()<<"recv client msg error: "<<UDT::getlasterror_desc()<<"ERROR code"<<UDT::getlasterror_code();
        return ;
    }
    if(recv_msg.unpack(buf, ret) < 0)//解包失败
    {
        QLOG_ERROR() << "parse msg fail";
        return ;
    }

    auto& msg = recv_msg.ebcMsg;
    // QLOG_WARN()<<"type:"<<msg.type();
    switch (msg.type())
    {
    case  config::MsgType::GET_NODE :
    {
        config::EbcNodes nodes;  //此处到复制到最终的msg里面可能存在多次拷贝，可能存在性能消耗，后期关注一下
        //作为服务端时，就从客户端节点取节点信息
        if(this->isServer)//this指针来访问自己的地址,如果服务器收到GET_NODE
        {
            struct sockaddr_in clientInfo;//客户端的地址
            uint32_t nat = parNat(msg.nodes().ebcnodes(0).port_nat());//节点端口(网络字节序)和NAT类型
            int addr_len = sizeof(clientInfo);
            UDT::getpeername(sock, (struct sockaddr *)&clientInfo, &addr_len);//存入clientInfo
            memset(buf, 0, BUFSIZE);
            config::EbcNode punch_node;
            msgPack send_msg(self.getId());//msgPack::msgPack(const NET::NodeId &_id):self_id(_id)
            //这个地方太混乱了，原因是客户端没有真正的发送起外网的IP和端口，以及NAT类型与PORT的组合处理比较麻烦
            punch_node.set_ip(clientInfo.sin_addr.s_addr);
            punch_node.set_port_nat(comPortNat(clientInfo.sin_port, nat));
            punch_node.set_id(msg.src_id());
            //             pack(config::MsgType type, void *msg, void *buf, int size,config::MsgSubType subType,const NET::NodeId dstId)
            ret = send_msg.pack(config::MsgType::PUNCH, &punch_node, buf, BUFSIZE);
            if(ret < 0)
                break;
            //开始查找节点并发送打洞信息
            NodeId cli_id(msg.src_id());
            if((kad->getNode(cli_id).get() != nullptr) && (!kad->getNode(cli_id)->isExpired()))
                break;

            std::list<Sp<Node>> sendnodes = kad->repNodes(cli_id);
            QLOG_WARN() << "rep node num= " << sendnodes.size();
            for(auto &node : sendnodes)
            {
                auto tmp = nodes.add_ebcnodes();
                tmp->set_id(&node->getId(), ID_LENGTH);
                tmp->set_ip(node->getAddr().getIPv4().sin_addr.s_addr);
                tmp->set_port_nat(comPortNat(node->getAddr().getIPv4().sin_port, node->getNatType()));
                ret = UDT::sendmsg(node->getSock(), buf, ret);
                QLOG_WARN() << "PUNCH ret = " << ret;
                node->getId().printNodeId(1);
            }
            //把对方节点加入服务器的K桶
            Sp<Node> clinode = std::make_shared<Node> (cli_id);
            clinode->setAddr((struct sockaddr*)&clientInfo);
            clinode->setSock(sock);
            sockNodePairSrv[sock] = clinode;
            appendBucket(clinode);
        }
        else //peer收到GET_NODE后的动作
        {
            struct sockaddr_in peerInfo;//对端节点的信息
            uint32_t nat = parNat(msg.nodes().ebcnodes(0).port_nat());//节点端口(网络字节序)和NAT类型
            int addr_len = sizeof(peerInfo);
            UDT::getpeername(sock, (struct sockaddr *)&peerInfo, &addr_len);
            memset(buf, 0, BUFSIZE);
            config::EbcNode punch_node;
            msgPack send_msg(self.getId());
            punch_node.set_ip(peerInfo.sin_addr.s_addr);
            punch_node.set_port_nat(comPortNat(peerInfo.sin_port, nat));
            punch_node.set_id(msg.src_id());
            ret = send_msg.pack(config::MsgType::PUNCH, &punch_node, buf, BUFSIZE);
            if(ret < 0)
                break;

            //开始查找节点并发送打洞信息
            NodeId targetId(msg.nodes().ebcnodes(0).id());

            //此处需要修改，查找当间cli_id所在桶的所有节点
            auto targetnodes = kad->findClosestNodes(targetId, 8);
            for (auto &node : targetnodes)
            {
                //filter sender node in the reply list
                if(node->getId() == NodeId(msg.src_id()))
                    continue;

                auto tmp = nodes.add_ebcnodes();
                tmp->set_id(&node->getId(), ID_LENGTH);
                tmp->set_ip(node->getAddr().getIPv4().sin_addr.s_addr);
                tmp->set_port_nat(comPortNat(node->getAddr().getIPv4().sin_port, node->getNatType()));
                UDT::sendmsg(node->getSock(), buf, ret);
            }
        }

        memset(buf, 0, BUFSIZE);
        ret = recv_msg.pack(config::MsgType::REP, &nodes, buf, BUFSIZE, config::MsgSubType::NODE); //NodeId 没有传，因为string 转array没实现，注意!!
        if(ret < 0)
            return ;
        UDT::sendmsg(sock, buf, ret);
        break;
    }

    //收到查询命令，找到该节点，或者返回3个最近的节点
    case  config::MsgType::GET_DATA :
    {
        QLOG_WARN() << "RCV GET_DATA";
        config::search datanodes;//返回结果
        auto msgNode = kad->getNode(msg.src_id());//发送消息的节点
        memset(buf, 0, BUFSIZE);
        config::EbcNode punch_node;
        msgPack send_msg(self.getId());
        punch_node.set_ip(msgNode->getAddr().getIPv4().sin_addr.s_addr);
        punch_node.set_port_nat(comPortNat(msgNode->getAddr().getIPv4().sin_port, msgNode->getNatType()));
        punch_node.set_id(msg.src_id());
        ret = send_msg.pack(config::MsgType::PUNCH, &punch_node, buf, BUFSIZE);
        if(ret < 0)
            break;
        //config::EbcNode searchernode;
        config::EbcNodes nodes;
        NodeId searchId(msg.msg().tid());
        datanodes.set_tid(msg.msg().tid());
        if(kad->findNode(searchId))//找到data
        {
             auto node = kad->getNode(searchId);
            auto tmp = nodes.add_ebcnodes();
            tmp->set_id(&(node->getId()), ID_LENGTH);
            tmp->set_ip(node->getAddr().getIPv4().sin_addr.s_addr);
            tmp->set_port_nat(comPortNat(node->getAddr().getIPv4().sin_port, node->getNatType()));
            UDT::sendmsg(node->getSock(), buf, ret);
        }
        else
        {
            auto targetnodes = kad->findClosestNodes(searchId, 3);//查找最近的3个节点回复回去
            QLOG_ERROR() << "REP closest node number =" << targetnodes.size();

            for (auto &node : targetnodes)
            {
                if(node->getId() == msgNode->getId())
                    continue;//如果找到的id和发送端相同则过滤掉,不能自己给自己打洞.
                QLOG_WARN()<<"REP the node:";
                node->getId().printNodeId(1);
                auto tmp = nodes.add_ebcnodes();
                tmp->set_id(&(node->getId()), ID_LENGTH);
                tmp->set_ip(node->getAddr().getIPv4().sin_addr.s_addr);
                tmp->set_port_nat(comPortNat(node->getAddr().getIPv4().sin_port, node->getNatType()));
                UDT::sendmsg(node->getSock(), buf, ret);
            }
        }
        datanodes.mutable_nodes()->CopyFrom(nodes);
        memset(buf, 0, BUFSIZE);
        ret = recv_msg.pack(config::MsgType::REP, &datanodes, buf, BUFSIZE, config::MsgSubType::DATA);
        if(ret < 0)
            return ;
        UDT::sendmsg(sock, buf, ret);
        break;
    }
    case config::MsgType::REP ://此处需要把接收的服务器发来的节点进行打洞  此处不需要改动
    {
        if(config::MsgSubType::NODE == msg.sub_type())
        {
            config::EbcNodes nodes = msg.nodes();
            config::EbcNode node;
            int node_count = nodes.ebcnodes_size();
            // QLOG_WARN()<<"get reply nodes number :"<<node_count;
            for(int i = 0; i < node_count; ++i)
            {
                node = nodes.ebcnodes(i);
                NodeId tId{node.id()};
                //filter blacklist
                auto herenode = std::find_if(blacklist.begin(), blacklist.end(),
                                         [&tId](std::list<Sp<Node>>::value_type & blacknode)
                {
                    return blacknode->getId() == tId;
                });
                if(herenode != blacklist.end())
                    continue;
                //filter self node in reply
                if(tId == self.getId())
                {
                    QLOG_WARN() << "filter self success!";
                    continue;
                }

                //开始UDT的打洞
                UDTSOCKET sock = UDT::INVALID_SOCK;
                if(kad->findNode(tId))
                {
                    if(!kad->getNode(tId)->isExpired())//如何桶中已发现该节点
                        continue;
                }
                else
                {
                    auto here = std::find_if(sockNodePair.begin(), sockNodePair.end(),
                                             [&tId](std::map<UDTSOCKET, Sp<Node>>::value_type & socknodes)
                    {
                        return socknodes.second->getId() == tId;
                    });
                    if(here != sockNodePair.end())
                        continue;
                }
                sock = startPunch(epollFd, node.ip(), parPort(node.port_nat()));
                // QLOG_ERROR()<<"REP_NODE SOCK="<<sock;
                if(sock == UDT::INVALID_SOCK)
                    continue;
                Sp<Node> lNode = std::make_shared<Node>(tId);
                lNode->setSock(sock);
                struct sockaddr_in peer_addr;
                peer_addr.sin_addr.s_addr = node.ip();
                peer_addr.sin_port = parPort(node.port_nat());
                peer_addr.sin_family = AF_INET;
                lNode->setAddr((struct sockaddr*)&peer_addr);
                sockNodePair[sock] = lNode;
            }
        }
        else if(config::MsgSubType::DATA == msg.sub_type())
        {
            QLOG_WARN() << "!!!!!!!!!!"; //1
            config::search sr = msg.msg();
            config::EbcNode node;
            NodeId tid(sr.tid());
            int node_count = sr.nodes().ebcnodes_size();
            auto tidsr = srch->findSearchList(tid);
            if(kad->findNode(tid))//TEST
            {
                tidsr->done = true;
                break;
            }
            if(tidsr == srch->searches.end())//正常来说不会出现
            {
                QLOG_ERROR() << "the searchlist is not exist";
                return ;
            }
            //QLOG_WARN() << "1 check done" << tidsr->done;
            if(tidsr->done)
                break;
            /***** find the reply node and set some properties *****/
            NodeId srcId(msg.src_id());
            srcId.printNodeId();//2
            for(auto&n : tidsr->searchNodes)
            {
                if(n.node->getId() == srcId)
                {
                    //QLOG_ERROR()<<"replie=1";
                    //srcId.printNodeId(1);
                    n.replied = 1;
                    break;
                }
            }
            bool haveSent = false;
            for(int i = 0; i < node_count; i++)
            {
                node = sr.nodes().ebcnodes(i);
//                QLOG_ERROR() << "rep_data list:"<<i; //3
                NodeId nodeId(node.id());
//                nodeId.printNodeId(true);//4
                /***** find if the node in the bucket *****/
                auto n = kad->getNode(nodeId);
                if((n.get() != nullptr) && (!n->isExpired())) //think about it :if the node has relay to add bucket
                {
//                    QLOG_ERROR()<<"the rep node in the bucket:";
//                    n->getId().printNodeId(1);//TEST
                    srch->addSearchNode(n, tid);
                    if(!haveSent)
                    {
                        for(auto& sn : tidsr->searchNodes)
                        {
                            /// QLOG_WARN()<<"707required times = "<<sn.requiredTimes<<"replied = "<<sn.replied;
                            if((!sn.replied) && (sn.requiredTimes < 3) && (sn.requestTime < (clock::now() - seconds(2))))
                            {
                                QLOG_WARN() << "next";
                                sn.node->getId().printNodeId();
                                sendSearchNode(sn.node, tid);
                                sn.requestTime = clock::now();
                                sn.requiredTimes += 1;
                                haveSent = true;
                                break;
                            }
                        }
                    }
                    continue;
                }
                /***** if the node is in blacklist *****/
                auto herenode = std::find_if(blacklist.begin(), blacklist.end(),
                                         [&nodeId](std::list<Sp<Node>>::value_type & blacknode)
                {
                    return blacknode->getId() == nodeId;
                });
                if(herenode != blacklist.end())
                {
                    continue;
                }
                /***** if the node is punching *****/
                auto here = std::find_if(sockNodePair.begin(), sockNodePair.end(),
                                         [&nodeId](std::map<UDTSOCKET, Sp<Node>>::value_type & socknodes)
                {
                    return socknodes.second->getId() == nodeId;
                });
                if(here != sockNodePair.end())
                {
                    idTidPair[nodeId] = tid;
                    continue;
                }

                idTidPair[nodeId] = tid;
                UDTSOCKET sock = UDT::INVALID_SOCK;
                sock = startPunch(epollFd, node.ip(), parPort(node.port_nat()));
                if(sock == UDT::INVALID_SOCK)
                {
                    QLOG_WARN()<<"SOCK INVALID";
                    continue;
                }
                Sp<Node> lNode = std::make_shared<Node>(node.id());
                lNode->setSock(sock);
                struct sockaddr_in peer_addr;
                peer_addr.sin_addr.s_addr = node.ip();
                peer_addr.sin_port = parPort(node.port_nat());
                peer_addr.sin_family = AF_INET;
                lNode->setAddr((struct sockaddr*)&peer_addr);
                sockNodePair[sock] = lNode;
                if(nodeId == tid)
                {
                    tidsr->done = true;
                    //QLOG_WARN()<<"2 set done true"<<tidsr->done;
                    auto test = srch->findSearchList(tid);
                    QLOG_WARN() << "3 rep get the tid ,set done = " << test->done;
//                    test->tid.printNodeId();
                    foundCallback(tid, *lNode);
                    break;
                }

                //call futher search
                if(!haveSent)
                {
                    for(auto& sn : tidsr->searchNodes)
                    {
                        //QLOG_WARN()<<"763required times = "<<sn.requiredTimes<<"replied = "<<sn.replied;
                        if((!sn.replied) && (sn.requiredTimes < 3) && (sn.requestTime < (clock::now() - seconds(2))))
                        {
                            QLOG_WARN() << "next";
                            sn.node->getId().printNodeId();
                            sendSearchNode(sn.node, tid);
                            sn.requestTime = clock::now();
                            sn.requiredTimes += 1;
                            haveSent = true;
                            break;
                        }
                    }
                }
            }
        }
        break;
    }
    case config::MsgType::PUNCH:
    {
        config::EbcNodes nodes = msg.nodes();
        config::EbcNode node;
        node = nodes.ebcnodes(0);
        //开始UDT的打洞
        UDTSOCKET sock = UDT::INVALID_SOCK;
        NodeId punchId{node.id()};
        /***** if the punchnode is in blacklist *****/
        auto herenode = std::find_if(blacklist.begin(), blacklist.end(),
                                 [&punchId](std::list<Sp<Node>>::value_type & blacknode)
        {
            return blacknode->getId() == punchId;
        });
        if(herenode != blacklist.end())
            break;
        /***** if the punchnode is in bucket *****/
        if(kad->findNode(punchId))
        {
            break;
        }
        else
        {
            auto here = std::find_if(sockNodePair.begin(), sockNodePair.end(),
                                     [&punchId](std::map<UDTSOCKET, Sp<Node>>::value_type & socknodes)
            {
                    return socknodes.second->getId() == punchId;
            });
            if(here != sockNodePair.end())
            {
                break;
            }
        }
        sock = startPunch(epollFd, node.ip(), parPort(node.port_nat()));
        if(sock == UDT::INVALID_SOCK)
            break;
        Sp<Node> lNode = std::make_shared<Node>(punchId);
        lNode->setSock(sock);
        struct sockaddr_in peer_addr;
        peer_addr.sin_addr.s_addr = node.ip();
        peer_addr.sin_port = parPort(node.port_nat());
        peer_addr.sin_family = AF_INET;
        lNode->setAddr((struct sockaddr*)&peer_addr);
        sockNodePair[sock] = lNode;
        //此处是否也需要将对方先加入自己桶
    }
    break;
    case config::MsgType::SENDDATASTREAM:
    {
        auto& tmpStr = msg.ebcdata();
        userData.emplace_back(tmpStr);
//        QLOG_INFO()<<"L884 receive msg len: "<<tmpStr.size()<<tmpStr.c_str();
//        QLOG_INFO()<<"userData size:"<<getUserDataListSize();
        break;
    }
    default:
        break;
    }
}

int NetEngine::startPunch(int& epollFd, uint32_t ip, uint16_t port)//对端的IP和PORT
{

    UDTSOCKET sock = UDT::socket(AF_INET, SOCK_DGRAM, 0);
    // QLOG_ERROR()<<"PUNCH SOCK="<<sock;
    if(UDT::INVALID_SOCK == sock)
    {
        QLOG_ERROR() << "create new UDT sock error: " << UDT::getlasterror_desc();
        return UDT::INVALID_SOCK;
    }

    struct sockaddr_in peer_addr;
    peer_addr.sin_addr.s_addr = ip;
    peer_addr.sin_port = port;
    peer_addr.sin_family = AF_INET;

    bool rendezvous = true;
    UDT::setsockopt(sock, 0, UDT_RENDEZVOUS, &rendezvous, sizeof(bool));
    setUdtOpt(sock);

    if(UDT::bind(sock, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0)
    {
        QLOG_ERROR() << "UDT punch bind error" << UDT::getlasterror_desc() << " code : " << UDT::getlasterror_code();
        UDT::close(sock);
        return UDT::INVALID_SOCK;
    }

    if(UDT::connect(sock, (struct sockaddr*)&peer_addr, sizeof(peer_addr)) < 0)
    {
        QLOG_ERROR() << "UDT connect error" << UDT::getlasterror().getErrorMessage();
        UDT::close(sock);
        return UDT::INVALID_SOCK;
    }
    //在这里调用epoll_add似乎不太好，后期再关注一下
    UDT::epoll_add_usock(epollFd, sock, nullptr);
    return sock;
}
//加服务器K桶
//void NetEngine::appendBucket(const Sp<Node> &node)
bool NetEngine::appendBucket(const Sp<Node> &node)
{
    return kad->onNewNode(node, 2, isServer); //judge if appendbucket success
}


bool NetEngine::joinNetwork(const std::string joinNetworkNodeAddress)
{
    NET::NodeId sid{};
    sid = joinNetworkNodeAddress.substr(3, ID_LENGTH);
    return startSearch(sid);
}

bool NetEngine::getBucket(std::list<std::string> &_bucketList)
{
    for(auto &b : kad->buckets)
    {
        for(auto &n : b.nodes)
        {
            if(!n->isExpired())
            {
                _bucketList.push_back("ebc" + n->getId().toString());//这里后四位加什么
            }
        }
    }
}

bool NetEngine::eraseNode(const std::string breakNetworkNodeAddress)
{
    NodeId sid{};
    sid = breakNetworkNodeAddress.substr(3, ID_LENGTH);
    Sp<Node> node = kad->getNode(sid);
    if(node == nullptr)
        return false;
    blacklist.emplace_back(node);//添加进黑名单
    int sock = node->getSock();
    node->setExpired();
    UDT::epoll_remove_usock(epollFd, sock);
    UDT::close(sock);
    setNodeExpired(sock);
    eraseNodeExpired(sock);
    return true;
}

bool NetEngine::eraseNode(NodeId tId)
{

    QLOG_ERROR()<<"delete the node and add to blacklist:";
    tId.printNodeId();
    Sp<Node> node = kad->getNode(tId);
    if(node == nullptr)
        return false;
    blacklist.emplace_back(node);
    int sock = node->getSock();
   // node->setExpired();
    UDT::epoll_remove_usock(epollFd, sock);
    UDT::close(sock);
    setNodeExpired(sock);
    eraseNodeExpired(sock);
    return true;
}

bool NetEngine::sendDataStream(const std::string targetNodeAddress, const char *sendDataStreamBuffer, const uint32_t sendDataStreamBufferSize)
{
    if (sendDataStreamBufferSize > 1024 * 1024)
    {
        QLOG_ERROR() << "Datasize out of range";
        return false;
    }
    NodeId tid(targetNodeAddress.substr(3, ID_LENGTH));
    tid.printNodeId(true);

    sendUserData(tid, sendDataStreamBuffer, sendDataStreamBufferSize);
}

int NetEngine::getUserDataListSize()
{
//    if(userData.empty())
//        return 0;
    return userData.size();
}

void NetEngine::setNodeExpired(const UDTSOCKET &sock, bool isServer)
{
    auto iter = isServer ? sockNodePairSrv.find(sock) : sockNodePair.find(sock);
    if(iter != (isServer ? sockNodePairSrv.end() : sockNodePair.end()))
    {
        Sp<Node>& epnode = iter->second;
        epnode->setExpired();
    }
}

void NetEngine::eraseNodeExpired(const UDTSOCKET &sock, bool isServer)
{
    if(isServer)
    {
        auto iter = sockNodePairSrv.find(sock);
        if(iter != sockNodePairSrv.end())
            sockNodePairSrv.erase(sock);
    }
    else
    {
        auto iter = sockNodePair.find(sock);
        if(iter != sockNodePair.end())
            sockNodePair.erase(sock);
    }
}

bool NetEngine::getUserDate(std::string &data)
{
    if(userData.empty())
        return false;

    data = userData.front();
    userData.pop_front();
}
}

