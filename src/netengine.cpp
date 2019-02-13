#include "netengine.h"
#include <iostream>
#include <cstring>

namespace NET
{
NetEngine::NetEngine(const NodeId _id, const bool _isServer):self(_id), isServer(_isServer)
{
   local_addr.sin_addr.s_addr = INADDR_ANY;
   local_addr.sin_port = 0;
   local_addr.sin_family = AF_INET;
}

NetEngine::~NetEngine()
{

    boot_thread_flag = false;
    server_thread_flag = false;
    UDT::close(boot_sock);
    for(auto sock:peerNode)
        UDT::close(sock.second.getSock());
    for(auto sock:clientNode)
        UDT::close(sock.second.getSock());
}

void NetEngine::startServer()
{
   if(self.getId().empty())
   {
       std::cout<<"Server net start fail: Id is empty"<<std::endl;
       return ;
   }

   isServer = true;
   server_thread_flag = true;
   server = std::thread
   (  //lambda表达式，先不关注
   [&]()
   {
       std::cout<<"server thread : "<<std::this_thread::get_id()<<std::endl;
       //UDTSOCKET srv = UDT::socket(AF_INET, SOCK_STREAM, 0);
       UDTSOCKET srv = UDT::socket(AF_INET, SOCK_DGRAM, 0);
       UDTSOCKET cli;

        int epollFd = UDT::epoll_create();
        int event = 0 ;
        int ret = 0;
        std::set<UDTSOCKET> readfds;
        std::set<UDTSOCKET> errfds;
        event = UDT_EPOLL_IN|UDT_EPOLL_ERR;

        int sock_len = sizeof(struct sockaddr);
       struct sockaddr_in srv_addr, client;
       srv_addr.sin_addr.s_addr = INADDR_ANY;
       srv_addr.sin_port = htons(SRV_PORT);
       srv_addr.sin_family = AF_INET;

       setUdtOpt(srv);
       if(UDT::bind(srv, (struct sockaddr*)&srv_addr, sizeof(srv_addr)) < 0)
       {
            std::cout<<"UDT bind server error"<<UDT::getlasterror().getErrorMessage()<<std::endl;
            return ;
       }

       if(UDT::listen(srv, 10000) < 0)
       {
            std::cout<<"UDT listen server error"<<UDT::getlasterror().getErrorMessage()<<std::endl;
            return ;
       }

       UDT::epoll_add_usock(epollFd, srv, &event);
       while(server_thread_flag)
       {
           //这个wait只会监听srv这个socket以及由socket返回的客户端的socket;
           ret = UDT::epoll_wait(epollFd, &readfds, nullptr, &errfds, 5*1000);
           //ret = UDT::epoll_wait(epollFd, &readfds, nullptr, 5*1000);
           if(ret < 0)
           {
               std::cout<<"UDT epoll_wait error"<<UDT::getlasterror().getErrorMessage()<<std::endl;
               return ;
           }
           else if(ret == 0)
               continue;

           for(auto& sock:errfds)
           {
			   //客户端发消息过来
			   int state=0;
			   int len = sizeof(state);
			   UDT::getsockopt(sock, 0, UDT_STATE, &state, &len);
			   //对服务器来说没有CONNECTING状态，因为它不会去主动连接
			   if(CLOSED == state || BROKEN == state )
			   {
				   std::cout<<"client disconnected"<<std::endl;
				   UDT::epoll_remove_usock(epollFd, sock);
				   UDT::close(sock);
				   NetEngine::delClientNode(sock);
				   continue;
			   }
		   }
           for(auto& sock:readfds)
           {
               if(sock == srv) //客户端连接成功
               {
                   if((cli = UDT::accept(srv, (struct sockaddr*)&client, &sock_len)) < 0)
                   {
                       std::cout<<"UDT accept error"<<UDT::getlasterror().getErrorMessage()<<std::endl;
                       continue;
                   }

                   std::cout<<"peer info:"<<inet_ntoa(client.sin_addr)<<"@"<<ntohs(client.sin_port)<<std::endl;
				   std::cout<<"cli = "<<cli<<std::endl;
				   //这里要不要设置cli这个socket的属性，还是说cli会继承srv套接字的属性
                   UDT::epoll_add_usock(epollFd, cli, &event);
               }
               else
               {
                   //客户端发消息过来
                   int state=0;
                   int len = sizeof(state);
                   UDT::getsockopt(sock, 0, UDT_STATE, &state, &len);
				   //对服务器来说没有CONNECTING状态，因为它不会去主动连接
				   if(CLOSED == state || BROKEN == state )
                   {
                       std::cout<<"client disconnected"<<std::endl;
                       UDT::epoll_remove_usock(epollFd, sock);
                       UDT::close(sock);
                       delClientNode(sock);
                       continue;
                   }

                   //接收并解析消息
                    handleMsg(sock) ;

               }
           }
       }
       }
    );
   server.detach();
}

void NetEngine::startClient(const std::string ip, const uint16_t port)
{
   //boot_sock = UDT::socket(AF_INET, SOCK_STREAM, 0);
   boot_sock = UDT::socket(AF_INET, SOCK_DGRAM, 0);
   if(boot_sock < 0)
   {
       std::cout<<"UDT socket error"<<UDT::getlasterror().getErrorMessage()<<std::endl;
       return ;
   }

   struct sockaddr_in srv_addr;
   srv_addr.sin_addr.s_addr = inet_addr(ip.c_str());
   srv_addr.sin_port = htons(port);
   srv_addr.sin_family = AF_INET;

    setUdtOpt(boot_sock);

    if(UDT::bind(boot_sock, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0)
    {
       std::cout<<"UDT bind error"<<UDT::getlasterror().getErrorMessage()<<std::endl;
       return ;
    }

    int sock_len = sizeof(local_addr);
    if(UDT::getsockname(boot_sock, (struct sockaddr *)&local_addr, &sock_len) < 0)
    {
        std::cout<<"UDT getsockname error"<<UDT::getlasterror().getErrorMessage()<<std::endl;
        return ;
    }
    std::cout<<"local port = "<<ntohs(local_addr.sin_port)<<std::endl;


    boot_thread_flag = true;
    boot_thread = std::thread([&, srv_addr]()  //lambda 表达式
    //与服务器交互的线程
    {
        std::cout<<"boot thread : "<<std::this_thread::get_id()<<std::endl;

        if(UDT::connect(boot_sock, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) < 0)
        {
            std::cout<<"UDT boot socket connect error"<<UDT::getlasterror().getErrorMessage()<<std::endl;
            return ;
        }

        std::cout<<"srv info"<<inet_ntoa(srv_addr.sin_addr)<<"@"<<htons(srv_addr.sin_port)<<std::endl;
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
            //ret = UDT::epoll_wait(epollFd, &readfds, &writefds, 1*1000);
            ret = UDT::epoll_wait(epollFd, &readfds, &writefds, &errfds, 5*1000);
            //std::cout<<"ret = "<<ret<<std::endl;
            if(ret < -1)
            {
                std::cout<<"UDT epoll_wait error"<<UDT::getlasterror().getErrorMessage()<<std::endl;
                return ;
            }
            else if(ret == 0)
                continue;


            for(auto& sock:errfds)   //C++11 新特性
            {
                UDT::getsockopt(sock, 0, UDT_STATE, &state, &len);
                //std::cout<<"in errfds state = "<<state<<std::endl;
                UDT::epoll_remove_usock(epollFd, sock);
                if(sock == boot_sock)
                {
                    //如果是服务器连接失败，则做重连动作  TODO
                    std::cout<<"server connect error! re-connecting..."<<std::endl;
                    //if(UDT::connect(boot_sock, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) < 0)
                    //{
                    //	std::cout<<"UDT boot socket connect error"<<UDT::getlasterror().getErrorMessage()<<std::endl;
                    //	return ;
                    //}
                    //event = UDT_EPOLL_OUT|UDT_EPOLL_ERR;
                    //UDT::epoll_add_usock(epollFd, sock, &event);
                }
                else
                {
                    std::cout<<"peer connect error"<<std::endl;
                    UDT::close(sock);
                    delPeerNode(sock);
                }
            }
            /*可写，这里表示connect连接状态, 在套接字可写后，获取套接字的状态，根据不同的状态可以知道socket的连接状态*/
            for(auto& sock:writefds)   //C++11 新特性
            {
                UDT::getsockopt(sock, 0, UDT_STATE, &state, &len);
                //std::cout<<"in writefds state = "<<state<<std::endl;
                //connect 连接失败了
                if(CLOSED == state || BROKEN == state || CONNECTING == state)
                {
                    UDT::epoll_remove_usock(epollFd, sock);
                    if(sock == boot_sock)
                    {
                        //如果是服务器连接失败，则做重连动作
                      std::cout<<"server connect error! re-connecting..."<<std::endl;
                      if(UDT::connect(boot_sock, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) < 0)
                      {
                          std::cout<<"UDT boot socket connect error"<<UDT::getlasterror().getErrorMessage()<<std::endl;
                          return ;
                      }
                      event = UDT_EPOLL_OUT|UDT_EPOLL_ERR;
                      UDT::epoll_add_usock(epollFd, sock, &event);
                    }
                    else
                    {
                      std::cout<<"peer connect error"<<std::endl;
                        UDT::close(sock);
                      delPeerNode(sock);
                    }
                    continue;
                }
                //否则就是连接成功了
                //把监听可写状态移除
                event = UDT_EPOLL_IN|UDT_EPOLL_ERR;
                UDT::epoll_remove_usock(epollFd, sock);
                UDT::epoll_add_usock(epollFd, sock, &event);

                if(sock == boot_sock) //服务器连接成功
                {
                   //发送findNode命令到服务器
                    config::EbcNode self_node;
                    self_node.set_port_nat(comPortNat(0, self.getNatType()));
                    msgPack sendMsg(self.getId());
                    int msg_len = sendMsg.pack(config::MsgType::GET_NODE, &self_node, buf, sizeof(buf));
                    if(msg_len < 0)
                        continue;

                    std::cout<<"msg len = "<<msg_len;
                    //memset(buf, 0, sizeof(buf));
                    //sprintf(buf, "hello");
                    //std::cout<<"  send len"<<UDT::send(sock, buf, msg_len, 0)<<std::endl;
                    std::cout<<"  send len"<<UDT::sendmsg(sock, buf, msg_len)<<std::endl;
                }
                else
                {
                    //连接对端节点成功，则将改节点的socket记录
                    std::cout<<"peer node connect success!"<<std::endl;
                    NodeId peerId = sockIdPair[sock];
                    Node node;
                    //这里想找出此节点并更新其状态，但是可能会出问题，因为是引用
                    findPeerNode(peerId, node);
                    Node::NodeState state=Node::NodeState::CONNECTED;
                    node.setState(state);
                    updatePeerNode(peerId, node);
                    Node::printNode(node);
                }
            }

            /*有消息达到*/
            for(auto& sock:readfds)
            {
                UDT::getsockopt(sock, 0, UDT_STATE, &state, &len);
                //std::cout<<"in readfds state = "<<state<<std::endl;
                /*这个时候的失败一般是之前连接上了，但是中间连接断开了*/
                if(CLOSED == state || BROKEN == state || CONNECTING == state)
                {
                    UDT::epoll_remove_usock(epollFd, sock);
                    if(sock == boot_sock)
                    {
                        //如果是服务器连接失败，则做重连动作
                      std::cout<<"server connect error! re-connecting..."<<std::endl;
                      if(UDT::connect(boot_sock, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) < 0)
                      {
                          std::cout<<"UDT boot socket connect error"<<UDT::getlasterror().getErrorMessage()<<std::endl;
                          return ;
                      }
                      event = UDT_EPOLL_OUT|UDT_EPOLL_ERR;
                      UDT::epoll_add_usock(epollFd, sock, &event);
                    }
                    else
                    {
                      std::cout<<"client connect error"<<std::endl;
                        UDT::close(sock);
                      delPeerNode(sock);
                    }
                    continue;
                }

                handleMsg(sock, epollFd) ;
            }

        }
    });
    boot_thread.detach();

}

void NetEngine::printNodesInfo(int type)
{
    std::cout<<"PEER INFO :"<<std::endl;
    if(type == 1)
    {
        std::cout<<"online count = "<<peerNode.size()<<std::endl;
        for(auto &node:peerNode)
            Node::printNodeId(node.second.getId());

        return ;
    }
    for(auto &node:peerNode)
    {
        Node::printNode(node.second);
    }

    if(isServer)
    {
        std::cout<<"SELF IS A SERVER"<<std::endl<<"CLIENT INFO"<<std::endl;
        for(auto &node:clientNode)
        {
            Node::printNode(node.second);
        }
    }
}

void NetEngine::sendHello()
{
    std::thread test = std::thread([&]()
    {
        while(boot_thread_flag)
        {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            msgPack helloMsg(self.getId());
            char hello_buf[64]="";
            char msg_buf[1024]="";
            static int count = 0;
            sprintf(hello_buf, "hello count = %d", count++);
            for(auto it:peerNode)
            {
                auto& peer = it.second;
                int ret = helloMsg.pack(config::MsgType::REP, hello_buf, msg_buf, sizeof(msg_buf), config::MsgSubType::DATA, peer.getId());
                UDT::sendmsg(peer.getSock(), msg_buf, ret);
                //UDT::send(peer.getSock(), msg_buf, ret, 0);
            }
        }

    });

    test.detach();
}

void NetEngine::setUdtOpt(const UDTSOCKET &sock)
{
    bool opt = false;
    if(UDT::setsockopt(sock, 0, UDT_SNDSYN, &opt, sizeof(opt)) < 0)
    {
        std::cout<<"UDT set SNDSYN fail"<<UDT::getlasterror().getErrorMessage()<<std::endl;
        return ;
    }

    if(UDT::setsockopt(sock, 0, UDT_RCVSYN, &opt, sizeof(opt)) < 0)
    {
        std::cout<<"UDT set RCVSYN fail"<<UDT::getlasterror().getErrorMessage()<<std::endl;
        return ;
    }

    int size = 2*1024*1024;
    if(UDT::setsockopt(sock, 0, UDT_SNDBUF, &size, sizeof(size)) < 0)
    {
        std::cout<<"UDT set SNDBUF fail"<<UDT::getlasterror().getErrorMessage()<<std::endl;
        return ;
    }

    if(UDT::setsockopt(sock, 0, UDT_RCVBUF, &size, sizeof(size)) < 0)
    {
        std::cout<<"UDT set RCVBUF fail"<<UDT::getlasterror().getErrorMessage()<<std::endl;
        return ;
    }
}

int NetEngine::getReadableByte(const UDTSOCKET &sock)
{
    int count = -1;
    int len = sizeof(count);
    if(UDT::getsockopt(sock, 0, UDT_RCVDATA, &count, &len) < 0)
    {
        std::cout<<"UDT get RCVDATA fail"<<UDT::getlasterror().getErrorMessage()<<std::endl;
        return -1;
    }

    return count;
}

void NetEngine::handleMsg(UDTSOCKET sock, int epollFd)
{
    char buf[6*1024]="";
    msgPack recv_msg(self.getId());
    //int ret = UDT::recv(sock, buf, sizeof(buf), 0);
    int ret = UDT::recvmsg(sock, buf, sizeof(buf));
    std::cout<<"recv ret = "<<ret<<std::endl;
    if(ret<0)
    {
        std::cout<<"recv client msg error: "<<UDT::getlasterror_desc()<<"ERROR code"<<UDT::getlasterror_code()<<std::endl;
        return ;
    }
    if(recv_msg.unpack(buf, ret) < 0)
    {
        std::cout<<"parse msg fail"<<std::endl;
        return ;
    }

    auto& msg = recv_msg.ebcMsg;

    switch (msg.type()) {
    case  config::MsgType::GET_NODE :
    {
        config::EbcNodes nodes;  //此处到复制到最终的msg里面可能存在多次拷贝，可能存在性能消耗，后期关注一下
        //作为服务端时，就从客户端节点取节点信息
        if(this->isServer)
        {
            struct sockaddr_in clientInfo;
            uint32_t nat = parNat(msg.nodes().ebcnodes(0).port_nat());
            int addr_len = sizeof(clientInfo);
            UDT::getpeername(sock, (struct sockaddr *)&clientInfo, &addr_len);
            memset(buf, 0, sizeof(buf));
            config::EbcNode punch_node;
            msgPack send_msg(self.getId());
            //这个地方太混乱了，原因是客户端没有真正的发送起外网的IP和端口，以及NAT类型与PORT的组合处理比较麻烦
            punch_node.set_ip(clientInfo.sin_addr.s_addr);
            punch_node.set_port_nat(comPortNat(clientInfo.sin_port, nat));
            punch_node.set_id(msg.src_id());
            ret = send_msg.pack(config::MsgType::PUNCH, &punch_node, buf, sizeof(buf));
            if(ret < 0)
                break;
            for(auto &node:clientNode)

            {
                auto tmp = nodes.add_ebcnodes();
                tmp->set_id(&node.second.getId(), ID_LENGTH);
                tmp->set_ip(node.second.getAddr().getIPv4().sin_addr.s_addr);
                tmp->set_port_nat(comPortNat(node.second.getAddr().getIPv4().sin_port, node.second.getNatType()));

                //通知该节点：新节点上线
                UDT::sendmsg(node.second.getSock(), buf, ret);
                //UDT::send(node.second.getSock(), buf, ret, 0);
            }
            addClientNode(msg.src_id(), clientInfo, nat, sock);
        }
        else //作为节点，就从以连接节点取信息
        {}

        memset(buf, 0, sizeof(buf));
        ret = recv_msg.pack(config::MsgType::REP, &nodes, buf, sizeof(buf), config::MsgSubType::NODE); //NodeId 没有传，因为string 转array没实现，注意!!
        if(ret < 0)
            return ;
        UDT::sendmsg(sock, buf, ret);
        //UDT::send(sock, buf, ret, 0);

        break;
    }
    case config::MsgType::REP :
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
                sock = startPunch(epollFd, node.ip(), parPort(node.port_nat()));
                if(sock == UDT::INVALID_SOCK)
                    continue;
                addPeerNode(node.id(), node.ip(), node.port_nat(), sock);
            }
        }
        else if(config::MsgSubType::DATA == msg.sub_type())
        {
            //TODO
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
        sock = startPunch(epollFd, node.ip(), parPort(node.port_nat()));
        if(sock == UDT::INVALID_SOCK)
            break;
        addPeerNode(node.id(), node.ip(), node.port_nat(), sock);
    }
        break;
    default:
        break;
    }

}

int NetEngine::startPunch(int& epollFd, uint32_t ip, uint16_t port)
{
    UDTSOCKET sock = UDT::socket(AF_INET, SOCK_STREAM, 0) ;
    if(UDT::INVALID_SOCK == sock)
    {
        std::cout<<"create new UDT sock error: "<<UDT::getlasterror_desc()<<std::endl;
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
       std::cout<<"UDT punch bind error"<<UDT::getlasterror_desc()<<" code : "<<UDT::getlasterror_code()<<std::endl;
       UDT::close(sock);
       return UDT::INVALID_SOCK;
    }

    if(UDT::connect(sock, (struct sockaddr*)&peer_addr, sizeof(peer_addr)) < 0)
    {
        std::cout<<"UDT connect error"<<UDT::getlasterror().getErrorMessage()<<std::endl;
        UDT::close(sock);
        return UDT::INVALID_SOCK;
    }

    //在这里调用epoll_add似乎不太好，后期再关注一下
    UDT::epoll_add_usock(epollFd, sock, nullptr);
    return sock;
}

void NetEngine::addClientNode(const NodeId &_id,const uint32_t &ip,const uint32_t &port_nat,const UDTSOCKET& sock)
{
   NET::Node node(_id);
   SockAddr addr;
   Node::NatType nat = (Node::NatType)parNat(port_nat);
   addr.setPort(parPort(port_nat));
   addr.setIPv4(ip);

   node.setAddr(addr);
   node.setNat(nat);
   node.setSock(sock);

   clientNode.insert(std::make_pair(_id, node));
   //clientNode.insert(std::pair<NodeId, Node>(_id, node)); //这两种写法都可

   sockIdPair[sock] = _id;
}

void NetEngine::addClientNode(const std::string &_id, const struct sockaddr_in& _addr, const uint32_t &nat, const UDTSOCKET &sock)
{
    NodeId nid;
    Node::String2NodeId(_id, nid);
   NET::Node node(nid);
   SockAddr addr;
   addr.setPort(_addr.sin_port);
   addr.setIPv4(_addr.sin_addr.s_addr);

   node.setAddr(addr);
   node.setNat((const Node::NatType)nat);
   node.setSock(sock);
   Node::NodeState state=Node::NodeState::CONNECTED;
   node.setState(state);

   clientNode.insert(std::make_pair(nid, node));
   //clientNode.insert(std::pair<NodeId, Node>(_id, node)); //这两种写法都可

   sockIdPair[sock] = nid;
}

void NetEngine::addClientNode(const std::string &_id,const uint32_t &ip,const uint32_t &port_nat,const UDTSOCKET &sock)
{
    NodeId nid;
    Node::String2NodeId(_id, nid);

    addClientNode(nid, ip, port_nat, sock);
}

bool NetEngine::findClientNode(const NodeId &_id, Node &node)
{
    auto it = clientNode.find(_id);
    if(it == clientNode.end())
        return false;

    //node = *it;  //这个为什么不行？？？？
    node = clientNode[_id];

    return true;
}

bool NetEngine::findClientNode(const std::string &_id, Node &node)
{
   NodeId tmp;
   Node::String2NodeId(_id, tmp);
   findClientNode(tmp, node);
}

void NetEngine::delClientNode(const NodeId &_id)
{
    auto it = clientNode.find(_id);
    if(it != clientNode.end())
        clientNode.erase(it);
}

void NetEngine::delClientNode(const std::string &_id)
{
   NodeId tmp;
   Node::String2NodeId(_id, tmp);
   delClientNode(tmp);
}

void NetEngine::delClientNode(const UDTSOCKET &sock)
{
   auto it = sockIdPair.find(sock);
   if(it == sockIdPair.end())
       return ;
   delClientNode(it->second) ;
   sockIdPair.erase(it);
}

void NetEngine::addPeerNode(const NodeId &_id,const uint32_t &ip,const uint32_t &port_nat,const UDTSOCKET sock)
{
   NET::Node node(_id);
   SockAddr addr;
   Node::NatType nat = (Node::NatType)parNat(port_nat);
   addr.setPort(parPort(port_nat));
   addr.setIPv4(ip);

   node.setAddr(addr);
   node.setNat(nat);
   node.setSock(sock);

   peerNode.insert(std::make_pair(_id, node));
   sockIdPair[sock] = _id;
}

void NetEngine::addPeerNode(const std::string &_id,const uint32_t &ip,const uint32_t &port_nat,const UDTSOCKET sock)
{
    NodeId nid;
    Node::String2NodeId(_id, nid);

    addPeerNode(nid, ip, port_nat, sock);
}

bool NetEngine::findPeerNode(const NodeId &_id, Node &node)
{
    auto it = peerNode.find(_id);
    if(it == peerNode.end())
        return false;

    //node = *it;
    node = peerNode[_id];

    return true;
}

bool NetEngine::findPeerNode(const std::string &_id, Node &node)
{
   NodeId tmp;
   Node::String2NodeId(_id, tmp);
   findPeerNode(tmp, node);
}

void NetEngine::updatePeerNode(const NodeId &_id, const Node &node)
{
   peerNode[_id]  = node;
}

void NetEngine::delPeerNode(const NodeId &_id)
{
    auto it = peerNode.find(_id);
    if(it != peerNode.end())
        peerNode.erase(it);
}

void NetEngine::delPeerNode(const std::string &_id)
{

   NodeId tmp;
   Node::String2NodeId(_id, tmp);
   delPeerNode(tmp);
}

void NetEngine::delPeerNode(const UDTSOCKET &sock)
{
   auto it = sockIdPair.find(sock);
   if(it == sockIdPair.end())
       return ;
   delPeerNode(it->second) ;
   sockIdPair.erase(it);
}

}
