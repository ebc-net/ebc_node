﻿#include "netengine.h"
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
NetEngine::NetEngine(const NodeId _id,Sp<Bucket>_kad,const bool _isServer):self(_id), kad(_kad),isServer(_isServer)
{
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = 0;
    local_addr.sin_family = AF_INET;
    srch = std::make_shared<Search> (kad);
    sendSearchNode = [&](Sp<Node> &dstNode, NodeId tId)
    {
        QLOG_WARN()<<"send for search to Node:";
        dstNode->getId().printNodeId();
        char sbuf[1024]="";
        config::search searchNode;
        searchNode.set_isid(true);
        searchNode.set_tid(tId.toString());
        msgPack sendMsg(self.getId());
        int msg_len = sendMsg.pack(config::MsgType::GET_DATA, &searchNode, sbuf, sizeof(sbuf));//只传ID去
        if(msg_len < 0)
        {
            QLOG_ERROR()<<"SendSearchNode msglen error";
            return ;
        }
        UDT::sendmsg(dstNode->getSock(), sbuf, msg_len);
    };
}

NetEngine::~NetEngine()
{
    boot_thread_flag = false;
    server_thread_flag = false;
    UDT::close(boot_sock);
    kad->closeBucket([](int sock)
    {
        UDT::close(sock);
    });
}

void NetEngine::startServer()
{
    if(self.getId().empty())
    {
        QLOG_ERROR()<<"Server net start fail: Id is empty";
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
        event = UDT_EPOLL_IN|UDT_EPOLL_ERR;//Readable|Error

        int sock_len = sizeof(struct sockaddr);
        struct sockaddr_in srv_addr, client;
        srv_addr.sin_addr.s_addr = INADDR_ANY;
        srv_addr.sin_port = htons(SRV_PORT);
        srv_addr.sin_family = AF_INET;

        setUdtOpt(srv);
        if(UDT::bind(srv, (struct sockaddr*)&srv_addr, sizeof(srv_addr)) < 0)
        {
            QLOG_ERROR()<<"UDT bind server error"<<UDT::getlasterror().getErrorMessage();
            return ;
        }

        if(UDT::listen(srv, 10000) < 0)
        {
            QLOG_ERROR()<<"UDT listen server error"<<UDT::getlasterror().getErrorMessage();
            return ;
        }
        UDT::epoll_add_usock(epollFd, srv, &event);
        while(server_thread_flag)
        {
            //the wait only listen srv's socket and its return of client's socket
            ret = UDT::epoll_wait(epollFd, &readfds, nullptr, &errfds, 5*1000);

            if(ret < 0)
            {
                QLOG_ERROR()<<"UDT epoll_wait error"<<UDT::getlasterror().getErrorMessage();
                return ;
            }
            else if(ret == 0)
                continue;
            for(auto& sock:errfds)//Heartbeat fail!
            {
                //Error
                int state=0;
                int len = sizeof(state);
                UDT::getsockopt(sock, 0, UDT_STATE, &state, &len);
                //the srv has no state = CONNECTING ,as it should not connect.
                if(CLOSED == state || BROKEN == state )
                {
                    QLOG_INFO()<<"client disconnected";
                    UDT::epoll_remove_usock(epollFd, sock);
                    UDT::close(sock);
                    setNodeExpired(sock,true);
                }
            }
            for(auto& sock:readfds)//readable ,have msg come
            {
                if(sock == srv) //client connects success!
                {
                    if((cli = UDT::accept(srv, (struct sockaddr*)&client, &sock_len)) < 0)
                    {
                        QLOG_ERROR()<<"UDT accept error"<<UDT::getlasterror().getErrorMessage();
                        continue;
                    }

                    QLOG_INFO()<<"peer info:"<<inet_ntoa(client.sin_addr)<<"@"<<ntohs(client.sin_port);
                    QLOG_INFO()<<"cli = "<<cli;
                    //这里要不要设置cli这个socket的属性，还是说cli会继承srv套接字的属性
                    UDT::epoll_add_usock(epollFd, cli, &event);
                }
                else
                {
                    //msg of client
                    int state=0;
                    int len = sizeof(state);
                    UDT::getsockopt(sock, 0, UDT_STATE, &state, &len);//shut down
                    //the srv has no state = CONNECTING ,as it should not connect.
                    if(CLOSED == state || BROKEN == state )
                    {
                        QLOG_INFO()<<"client disconnected";
                        UDT::epoll_remove_usock(epollFd, sock);
                        UDT::close(sock);
                        setNodeExpired(sock,true);
                        continue;
                    }
                    //recieve and handle msg
                    handleMsg(sock) ;
                }
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
        QLOG_ERROR()<<"UDT socket error"<<UDT::getlasterror().getErrorMessage();
        return ;
    }

    struct sockaddr_in srv_addr;
    srv_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    srv_addr.sin_port = htons(port);
    srv_addr.sin_family = AF_INET;

    setUdtOpt(boot_sock);
    if(UDT::bind(boot_sock, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0)
    {
        QLOG_ERROR()<<"UDT bind error"<<UDT::getlasterror().getErrorMessage();
        return ;
    }

    int sock_len = sizeof(local_addr);
    if(UDT::getsockname(boot_sock, (struct sockaddr *)&local_addr, &sock_len) < 0)//获取本地地址，正确返回0。
    {
        QLOG_ERROR()<<"UDT getsockname error"<<UDT::getlasterror().getErrorMessage();
        return ;
    }
    QLOG_INFO()<<"local port = "<<ntohs(local_addr.sin_port);

    boot_thread_flag = true;
    boot_thread = std::thread([&, srv_addr]()  //lambda 表达式
                              //与服务器交互的线程
    {
        if(UDT::connect(boot_sock, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) < 0)
        {
            QLOG_ERROR()<<"UDT boot socket connect error"<<UDT::getlasterror().getErrorMessage();
            return ;
        }

        QLOG_INFO()<<"srv info"<<inet_ntoa(srv_addr.sin_addr)<<"@"<<htons(srv_addr.sin_port);
        char buf[1024]="";
        int epollFd = UDT::epoll_create();
        int event = 0 ;
        int ret = 0;
        int state=0;
        int len = sizeof(state);

        std::set<UDTSOCKET> readfds, writefds, errfds;

        /*监听套接字的可写，错误*/
        event = UDT_EPOLL_OUT|UDT_EPOLL_ERR;
        UDT::epoll_add_usock(epollFd, boot_sock, &event);

        while(boot_thread_flag)
        {
            ret = UDT::epoll_wait(epollFd, &readfds, &writefds, &errfds, 5*1000);

            //程序关闭时，直接退出
            if(!boot_thread_flag)
                return ;
            if(ret < -1)
            {
                QLOG_ERROR()<<"UDT epoll_wait error"<<UDT::getlasterror().getErrorMessage();
                return ;
            }

            for(auto& sock:errfds)
            {
                // UDT::getsockopt(sock, 0, UDT_STATE, &state, &len);
                //QLOG_INFO()<<"in errfds state = "<<state;
                UDT::epoll_remove_usock(epollFd, sock);
                if(sock == boot_sock)
                {
                    //如果是服务器连接失败，则做重连动作  TODO
                    //UDT有问题，不能连续连接 注意一下
                    QLOG_ERROR()<<"server connect error! re-connecting...";
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
                    QLOG_ERROR()<<"peer connect error1";
                    sockNodePair[sock]->getId().printNodeId();
                    UDT::close(sock);
                    setNodeExpired(sock);
                    sockNodePair.erase(sock);
                }
            }
            /*可写，这里表示connect连接状态, 在套接字可写后，获取套接字的状态，根据不同的状态可以知道socket的连接状态*/
            for(auto& sock:writefds)
            {
                event = UDT_EPOLL_IN|UDT_EPOLL_ERR;
                UDT::epoll_remove_usock(epollFd, sock);
                UDT::epoll_add_usock(epollFd, sock, &event);

                if(sock == boot_sock) //服务器连接成功
                {
                    //发送GET_NODE命令到服务器
                    config::EbcNode self_node;
                    self_node.set_port_nat(comPortNat(0, self.getNatType()));
                    msgPack sendMsg(self.getId());
                    int msg_len = sendMsg.pack(config::MsgType::GET_NODE, &self_node, buf, sizeof(buf));
                    if(msg_len < 0)
                        continue;
                    UDT::sendmsg(sock, buf, msg_len);
                }
                else
                {
                    //连接对端节点成功，则将改节点的socket记录,将此节点加入K桶
                    QLOG_INFO()<<"peer node connect success!";
                    Sp<Node>& node = sockNodePair[sock];////记录UDT套接字与节点ID的对应关系，方便通过UDTSOCKET快速找到ID
                    auto tid = idTidPair.find(node->getId());
                    if(tid != idTidPair.end())
                    {
                        srch->addSearchNode(node, tid->second);
                        srch->searchStep(tid->second, sendSearchNode,1);
                    }
                    appendBucket(node);//打洞成功的节点加入本地（客户端的）K桶简化
                }
            }
            /*有消息达到*/
            for(auto& sock:readfds)
            {
                UDT::getsockopt(sock, 0, UDT_STATE, &state, &len);
                /*这个时候的失败一般是之前连接上了，但是中间连接断开了*/
                if(CLOSED == state || BROKEN == state || CONNECTING == state)
                {
                    UDT::epoll_remove_usock(epollFd, sock);
                    QLOG_ERROR()<<"client connect error";

                    UDT::close(sock);
                    //失效
                    setNodeExpired(sock);
                    sockNodePair.erase(sock);
                    continue;
                }
                handleMsg(sock, epollFd) ;
            }

            //search time maintenance

            if(searchTime <= clock::now())
            {
                for (auto it = srch->searches.begin(); it !=srch->searches.end(); it++)
                {
                    if(!it->done && it->step_time + seconds(6) <= clock::now())
                        srch->searchStep(it, sendSearchNode,1);
                    time_point tm ;
                    if(!it->done)
                        tm = it->step_time + seconds(15);
                    if(tm < searchTime)
                        searchTime = tm;
                }
            }

            //bucket maintenance (expired node  or neighbourhood）
            auto sendFindNode = [&](Sp<Node> &dstNode, NodeId targetId)
            {
                config::EbcNode targetNode;
                targetNode.set_id(targetId.toString());
                msgPack sendMsg(self.getId());
                int msg_len = sendMsg.pack(config::MsgType::GET_NODE, &targetNode, buf, sizeof(buf));//只传ID去
                if(msg_len < 0)
                    return ;

                UDT::sendmsg(dstNode->getSock(), buf, msg_len);
            };
            if(clock::now() >= maintenanceTime)
            {
                if( kad->bucketMaintenance( sendFindNode,false ))//neighbour=false 桶维护
                {
                    maintenanceTime = clock::now()+ seconds(5);
                    QLOG_INFO()<<"send get_node for bucket maintenance";
                }
                else
                {
                    maintenanceTime = clock::now()+ seconds(60);

                    if(kad->grow_time >= clock::now() - seconds(150))//mybucket分裂后150s再扩桶
                    {
                        kad->bucketMaintenance(sendFindNode,true);//扩
                    }
                }
            }
        }
    });
    boot_thread.detach();

}

void NetEngine::startSearch(NodeId tId)
{
    auto callback = [&](NodeId tId, Node &sNode)
    {
        QLOG_WARN()<<"Find the search Node :";
        tId.printNodeId();
        QLOG_WARN()<<"Find in the Node:";
        sNode.getId().printNodeId();
    };
    srch->dhtSearch(tId,callback,sendSearchNode);
}

void NetEngine::setUdtOpt(const UDTSOCKET &sock)//srv
{
    bool opt = false;
    if(UDT::setsockopt(sock, 0, UDT_SNDSYN, &opt, sizeof(opt)) < 0)//etsockopt对套接字进行设置// if sending is blocking
    {
        QLOG_ERROR()<<"UDT set SNDSYN fail"<<UDT::getlasterror().getErrorMessage();
        return ;
    }

    if(UDT::setsockopt(sock, 0, UDT_RCVSYN, &opt, sizeof(opt)) < 0)
    {
        QLOG_ERROR()<<"UDT set RCVSYN fail"<<UDT::getlasterror().getErrorMessage();
        return ;
    }

    int size = 2*1024*1024;
    if(UDT::setsockopt(sock, 0, UDT_SNDBUF, &size, sizeof(size)) < 0)
    {
        QLOG_ERROR()<<"UDT set SNDBUF fail"<<UDT::getlasterror().getErrorMessage();
        return ;
    }

    if(UDT::setsockopt(sock, 0, UDT_RCVBUF, &size, sizeof(size)) < 0)
    {
        QLOG_ERROR()<<"UDT set RCVBUF fail"<<UDT::getlasterror().getErrorMessage();
        return ;
    }

#ifdef Q_OS_MAC

    int s_size = sizeof(int);
    if(UDT::getsockopt(sock, 0, UDP_RCVBUF, &size, &s_size) < 0)
    {
        QLOG_ERROR()<<"UDT set RCVBUF fail"<<UDT::getlasterror().getErrorMessage();
        return ;
    }
    QLOG_INFO()<<"udp buf size = "<<size;

    size = 2*1024*1024;
    if(UDT::setsockopt(sock, 0, UDP_RCVBUF, &size, sizeof(size)) < 0)
    {
        QLOG_ERROR()<<"UDT set RCVBUF fail"<<UDT::getlasterror().getErrorMessage();
        return ;
    }
#endif
}

void NetEngine::handleMsg(UDTSOCKET sock, int epollFd)//handleMsg(sock）
{
    memset(buf, 0, sizeof (buf));
    msgPack recv_msg(self.getId());
    int ret = UDT::recvmsg(sock, buf, sizeof(buf));
    if(ret<0)
    {
        QLOG_ERROR()<<"recv client msg error: "<<UDT::getlasterror_desc()<<"ERROR code"<<UDT::getlasterror_code();
        return ;
    }
    if(recv_msg.unpack(buf, ret) < 0)//解包失败
    {
        QLOG_ERROR()<<"parse msg fail";
        return ;
    }

    auto& msg = recv_msg.ebcMsg;

    switch (msg.type()) {
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
            memset(buf, 0, sizeof(buf));
            config::EbcNode punch_node;
            msgPack send_msg(self.getId());//msgPack::msgPack(const NET::NodeId &_id):self_id(_id)
            //这个地方太混乱了，原因是客户端没有真正的发送起外网的IP和端口，以及NAT类型与PORT的组合处理比较麻烦
            punch_node.set_ip(clientInfo.sin_addr.s_addr);
            punch_node.set_port_nat(comPortNat(clientInfo.sin_port, nat));
            punch_node.set_id(msg.src_id());
            //             pack(config::MsgType type, void *msg, void *buf, int size,config::MsgSubType subType,const NET::NodeId dstId)
            ret = send_msg.pack(config::MsgType::PUNCH, &punch_node, buf, sizeof(buf));
            if(ret < 0)
                break;
            //开始查找节点并发送打洞信息
            NodeId cli_id(msg.src_id());
            std::list<Sp<Node>> sendnodes = kad->repNodes(cli_id);
			QLOG_WARN()<<"rep node num= "<<sendnodes.size();
            for (auto &node:sendnodes)
            {
                auto tmp = nodes.add_ebcnodes();
                tmp->set_id(&node->getId(),ID_LENGTH);
                tmp->set_ip(node->getAddr().getIPv4().sin_addr.s_addr);
                tmp->set_port_nat(comPortNat(node->getAddr().getIPv4().sin_port,node->getNatType()));
                UDT::sendmsg(node->getSock(),buf,ret);
            }
            //把对方节点加入服务器的K桶
            Sp<Node> clinode = std::make_shared<Node> (cli_id);
            clinode->setAddr((struct sockaddr*)&clientInfo);
            clinode->setSock(sock);
            sockNodePairSrv[sock]= clinode;
            appendBucket(clinode);
        }
        else //peer收到GET_NODE后的动作
        {
            struct sockaddr_in peerInfo;//对端节点的信息
            uint32_t nat = parNat(msg.nodes().ebcnodes(0).port_nat());//节点端口(网络字节序)和NAT类型
            int addr_len = sizeof(peerInfo);
            UDT::getpeername(sock, (struct sockaddr *)&peerInfo, &addr_len);
            memset(buf, 0, sizeof(buf));
            config::EbcNode punch_node;
            msgPack send_msg(self.getId());
            punch_node.set_ip(peerInfo.sin_addr.s_addr);
            punch_node.set_port_nat(comPortNat(peerInfo.sin_port, nat));
            punch_node.set_id(msg.src_id());
            ret = send_msg.pack(config::MsgType::PUNCH, &punch_node, buf, sizeof(buf));
            if(ret < 0)
                break;

            //开始查找节点并发送打洞信息
            NodeId targetId(msg.nodes().ebcnodes(0).id());
            //此处需要修改，查找当间cli_id所在桶的所有节点
            auto targetnodes = kad->findClosestNodes(targetId,8);
            for (auto &node:targetnodes)
            {
                auto tmp = nodes.add_ebcnodes();
                tmp->set_id(&node->getId(),ID_LENGTH);
                tmp->set_ip(node->getAddr().getIPv4().sin_addr.s_addr);
                tmp->set_port_nat(comPortNat(node->getAddr().getIPv4().sin_port,node->getNatType()));
                UDT::sendmsg(node->getSock(),buf,ret);
            }
        }

        memset(buf, 0, sizeof(buf));
        ret = recv_msg.pack(config::MsgType::REP, &nodes, buf, sizeof(buf), config::MsgSubType::NODE); //NodeId 没有传，因为string 转array没实现，注意!!
        if(ret < 0)
            return ;
        UDT::sendmsg(sock, buf, ret);
        break;
    }

        //收到查询命令，找到该节点，或者返回3个最近的节点
    case  config::MsgType::GET_DATA :
    {
        config::search datanodes;//返回结果
        auto msgNode = kad->getNode(msg.src_id());
        memset(buf, 0, sizeof(buf));
        config::EbcNode punch_node;
        msgPack send_msg(self.getId());
        punch_node.set_ip(msgNode->getAddr().getIPv4().sin_addr.s_addr);
        punch_node.set_port_nat(comPortNat(msgNode->getAddr().getPort(), msgNode->getNatType()));
        punch_node.set_id(msg.src_id());
        ret = send_msg.pack(config::MsgType::PUNCH, &punch_node, buf, sizeof(buf));
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
            tmp->set_id(node->getId().toString());
            tmp->set_ip(node->getAddr().getIPv4().sin_addr.s_addr);
            tmp->set_port_nat(comPortNat(node->getAddr().getIPv4().sin_port,node->getNatType()));
            UDT::sendmsg(node->getSock(),buf,ret);
        }
        else
        {
            auto targetnodes = kad->findClosestNodes(searchId, 3);
            for (auto &node:targetnodes)
            {
                auto tmp = nodes.add_ebcnodes();
                tmp->set_id(&node->getId(),ID_LENGTH);
                tmp->set_ip(node->getAddr().getIPv4().sin_addr.s_addr);
                tmp->set_port_nat(comPortNat(node->getAddr().getIPv4().sin_port,node->getNatType()));
                UDT::sendmsg(node->getSock(),buf,ret);
            }
        }
        datanodes.mutable_nodes()->CopyFrom(nodes);
        memset(buf, 0, sizeof (buf));
        ret = recv_msg.pack(config::MsgType::REP, &datanodes, buf, sizeof(buf), config::MsgSubType::DATA);
        if(ret < 0)
            return ;
        UDT::sendmsg(sock,buf,ret);
        break;
    }
    case config::MsgType::REP ://此处需要把接收的服务器发来的节点进行打洞  此处不需要改动
    {
        if(config::MsgSubType::NODE == msg.sub_type())
        {
            config::EbcNodes nodes = msg.nodes();
            config::EbcNode node;
            int node_count = nodes.ebcnodes_size();
            for(int i=0; i<node_count; ++i)
            {
                node = nodes.ebcnodes(i);
                //开始UDT的打洞
                UDTSOCKET sock = UDT::INVALID_SOCK;
                NodeId tId{node.id()};
                if(kad->findNode(tId))
                {
                    if(!kad->getNode(tId)->isExpired())
                        continue;
                }
                else
                {
                    auto here = std::find_if(sockNodePair.begin(), sockNodePair.end(),
                                             [&tId](std::map<UDTSOCKET, Sp<Node>>::value_type &socknodes)
                    {
                            return socknodes.second->getId()== tId;
                });
                    if(here != sockNodePair.end())
                        continue;
                }
                sock = startPunch(epollFd, node.ip(), parPort(node.port_nat()));
                if(sock == UDT::INVALID_SOCK)
                    continue;
                Sp<Node> lNode = std::make_shared<Node>(tId);
                lNode->setSock(sock);
                struct sockaddr_in peer_addr;
                peer_addr.sin_addr.s_addr = node.ip();
                peer_addr.sin_port = parPort(node.port_nat());
                peer_addr.sin_family = AF_INET;
                lNode->setAddr((struct sockaddr*)&peer_addr);
                sockNodePair[sock]= lNode;
            }
        }
        else if(config::MsgSubType::DATA == msg.sub_type())
        {
            config::search sr = msg.msg();
            config::EbcNode node;
            NodeId tid(sr.tid());
            int node_count = sr.nodes().ebcnodes_size();
            auto tidsr = srch->findSearchList(sr.tid());
            if(tidsr == srch->searches.end())
            {
                QLOG_ERROR()<<"the searchlist is not exist";
                return ;
            }

            /***** find the reply node and set some properties *****/
            NodeId srcId(msg.src_id());
            for(auto&n :tidsr->searchNodes)
            {
                if(n.node->getId() == srcId)
                {
                    n.replied = 1;
                    break;
                }
            }

            for(int i = 0; i<node_count; i++)
            {
                node = sr.nodes().ebcnodes(i);
                /***** find if the node in the bucket *****/
                auto n = kad->getNode(node.id());
                if(n.get()!=nullptr && !n->isExpired())//think about it :if the node has relay to add bucket
                {
                    srch->addSearchNode(n,tid);
                    continue;
                }
                /***** if the node is punching *****/
                auto here = std::find_if(sockNodePair.begin(), sockNodePair.end(),
                                         [&node](std::map<UDTSOCKET, Sp<Node>>::value_type &socknodes)
                {
                        return socknodes.second->getId().toString()== node.id();
            });
                if(here != sockNodePair.end())
                {
                    idTidPair[node.id()] = sr.tid();
                    continue;
                }

                idTidPair[node.id()] = sr.tid();
                UDTSOCKET sock = UDT::INVALID_SOCK;
                sock = startPunch(epollFd, node.ip(),parPort(node.port_nat()));
                if(sock == UDT::INVALID_SOCK)
                    continue;

                if(node.id() == sr.tid())
                {
                    tidsr->done = true;
                    /*call callback function*/
                }

                //TODO call futher search
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
        NodeId tId{node.id()};
        if(kad->findNode(tId))
        {
            if(!kad->getNode(tId)->isExpired())
                break;
        }
        else
        {
            auto here = std::find_if(sockNodePair.begin(), sockNodePair.end(),
                                     [&tId](std::map<UDTSOCKET, Sp<Node>>::value_type &socknodes)
            {
                    return socknodes.second->getId()== tId;
        });
            if(here != sockNodePair.end())
                break;
        }
        sock = startPunch(epollFd, node.ip(), parPort(node.port_nat()));
        if(sock == UDT::INVALID_SOCK)
            break;
        Sp<Node> lNode = std::make_shared<Node>(tId);
        lNode->setSock(sock);
        struct sockaddr_in peer_addr;
        peer_addr.sin_addr.s_addr = node.ip();
        peer_addr.sin_port = parPort(node.port_nat());
        peer_addr.sin_family = AF_INET;
        lNode->setAddr((struct sockaddr*)&peer_addr);
        sockNodePair[sock]= lNode;
        //此处是否也需要将对方先加入自己桶
    }
        break;
    default:
        break;
    }
}

int NetEngine::startPunch(int& epollFd, uint32_t ip, uint16_t port)//对端的IP和PORT
{
    UDTSOCKET sock = UDT::socket(AF_INET, SOCK_DGRAM, 0);
    if(UDT::INVALID_SOCK == sock)
    {
        QLOG_ERROR()<<"create new UDT sock error: "<<UDT::getlasterror_desc();
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
        QLOG_ERROR()<<"UDT punch bind error"<<UDT::getlasterror_desc()<<" code : "<<UDT::getlasterror_code();
        UDT::close(sock);
        return UDT::INVALID_SOCK;
    }

    if(UDT::connect(sock, (struct sockaddr*)&peer_addr, sizeof(peer_addr)) < 0)
    {
        QLOG_ERROR()<<"UDT connect error"<<UDT::getlasterror().getErrorMessage();
        UDT::close(sock);
        return UDT::INVALID_SOCK;
    }
    //在这里调用epoll_add似乎不太好，后期再关注一下
    UDT::epoll_add_usock(epollFd, sock, nullptr);
    return sock;
}
//加服务器K桶
void NetEngine::appendBucket(const Sp<Node> &node)
{
    kad->onNewNode(node,2,isServer);
}

void NetEngine::setNodeExpired(const UDTSOCKET &sock,bool isServer)
{
    auto iter = isServer?sockNodePairSrv.find(sock):sockNodePair.find(sock);
    if(iter!=(isServer?sockNodePairSrv.end():sockNodePair.end()))
    {
        Sp<Node>& epnode = iter->second;
        epnode->setExpired();
    }
}
}

