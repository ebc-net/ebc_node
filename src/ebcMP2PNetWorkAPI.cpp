/* MP2P communication and network interface API/MP2P通讯与网络接口API函数
   File Name        : ebcMP2PNetWorkAPI.cpp
   Version          : 0.0.1
   Date             : 2019-03-23
   Interface design : Wang Yueping(version 0.0.1)
   Software         : Xu Xiaona,He Xiaoen,Sun jiarui(version 0.0.1)
   Language         : C++
   Development tool : Qt Creater 4.8.0
   Description      : MP2P communication and network interface API C++ file
 
  (C) Copyright 2019  Right Chain Technolegy Corp., LTD.
*/

#include "ebcMP2PNetWorkAPI.h"




// MP2P网络与通讯模块接口函数类
ebcMP2PNetWorkAPI::ebcMP2PNetWorkAPI()
{
}

ebcMP2PNetWorkAPI::~ebcMP2PNetWorkAPI()
{
}

/*
  功    能：按指定的组网节点创建MP2P网络
  输入参数：createNetworkNodeAddress-字符型数据，组网节点地址
  输出参数：无
  返回参数：true-创建成功，false-创建失败
*/
bool ebcMP2PNetWorkAPI::createNetwork(const std::string *createNetworkNodeAddress)
{
    NET::NodeId id{};
    id = createNetworkNodeAddress->substr(3, ID_LENGTH);
    NET::Sp<NET::Bucket>  kad = std::make_shared <NET::Bucket>(id);
    engine.NetInit(id, kad);
    engine.startClient();
    return 1;
}

/*
  功    能：按指定的组网节点刷新MP2P网络
  输入参数：createNetworkNodeAddress-字符型数据，组网节点地址
  输出参数：无
  返回参数：true-网络表有刷新，false-网络表无刷新
*/
bool ebcMP2PNetWorkAPI::updateNetwork()
{
    char buf[1024]="";
    auto sendFindNode = [&](NET::Sp<NET::Node> &dstNode, NET::NodeId targetId)
    {
        config::EbcNode targetNode;
        targetNode.set_id(targetId.toString());
        NET::msgPack sendMsg(engine.self.getId());
        int msg_len = sendMsg.pack(config::MsgType::GET_NODE, &targetNode, buf, sizeof(buf));//只传ID去
        if(msg_len < 0)
            return ;

        UDT::sendmsg(dstNode->getSock(), buf, msg_len);
    };
    engine.kad->bucketMaintenance(sendFindNode,true);
    return 1;
}

/*
  功    能：读取组网节点的MP2P网络表
  输入参数：createNetworkNodeAddress-字符型数据，组网节点地址
  输出参数：networkTable-结构数据，MP2P域网络表
  返回参数：无
*/
void ebcMP2PNetWorkAPI::getNetworkTable(NETWORK_DOMAIN_TABLE *networkTable)
{
    uint16_t i = 0;
    for(auto &b : engine.kad->buckets)
    {
        for(auto &n : b.nodes)
        {
            if(!n->isExpired())
            {
                networkTable->onlineNodeAddress.push_back("ebc" + n->getId().toString());//这里后四位加什么
                i++;
            }
        }
    }
    networkTable->onlineNodeNumber = i;
}

 /* 
  功    能：加入一个指定的节点到MP2P网络中
  输入参数：createNetworkNodeAddress-字符型数据，组网节点地址
            jionNetworkNodeAddress-字符型数据，指定加入网络的节点地址
  输出参数：无
  返回参数：true-节点加入成功，false-节点加入失败
  备    注：可以指定一个节点加入到网络中来
*/
bool ebcMP2PNetWorkAPI::joinNodeToNetwork(const std::string *joinNetworkNodeAddress)
{
    NET::NodeId sid{};
    sid = joinNetworkNodeAddress->substr(3, ID_LENGTH);
    return engine.startSearch(sid);
}

/*
  功    能：断开MP2P网络中的一个指定节点
  输入参数：createNetworkNodeAddress-字符型数据，组网节点地址
            breakNetworkNodeAddress-字符型数据，断开的网络节点地址
  输出参数：无
  返回参数：true-固定节点断开成功，false-固定节点断开失败
  备    注：可以指定一个网络中的节点断开
*/
bool ebcMP2PNetWorkAPI::breakNodeFormNetwork(const std::string *breakNetworkNodeAddress)
{
    NET::NodeId sid{};
    sid = breakNetworkNodeAddress->substr(3, ID_LENGTH);
    NET::Sp<NET::Node> node = engine.kad->getNode(sid);
    if(node!= nullptr)
    {
        int sock;
        for(auto &it : engine.sockNodePair)
        {
            if(it.second == node)
            {
                sock = it.first;
                break;
            }
        }
        node->setExpired();
        UDT::epoll_remove_usock(engine.epollFd, sock);
        UDT::close(sock);
        engine.setNodeExpired(sock,true);
        engine.eraseNodeExpired(sock,true);
        return true;
    }
    return false;

}

/*
  功    能：由源地址发送数据流到目标地址函数
  输入参数：sourceNodeAddress-字符型数据，发送数据源节点地址
            targetNodeAddress-字符型数据，接收数据目标节点地址，若目标地址为"ALL"则为广播式发送到当前网络域内的所有节点
            sendDataStreamBuffer-字符型数据流，发送数据流
            sendDataStreamBufferSize-整型数据，发送数据流长度
  输出参数：无
  返回参数：true-发送成功，false-发送失败
*/
bool ebcMP2PNetWorkAPI::sendDataStream(const std::string *sourceNodeAddress, const std::string *targetNodeAddress, const char *sendDataStreamBuffer, const uint32_t sendDataStreamBufferSize)
{
    if (sendDataStreamBufferSize > 1024*1024)
    {
        QLOG_ERROR()<<"Datasize out of range";
        return false;
    }
    NET::NodeId sid{},tid{};
    sid = sourceNodeAddress->substr(3,ID_LENGTH);
    tid = targetNodeAddress->substr(3,ID_LENGTH);
    if(sid != engine.self.getId())
    {
        QLOG_ERROR()<<"self id is wrong!";
        return false;
    }

    return 0;
}

/*
  功    能：从网络接收数据流中读取数据函数
  输入参数：receiveDataStreamBufferSize-整型数据，读取数据流长度
  输出参数：receiveDataStreamBuffer-字符型数据，读取的数据流数据
  返回参数：实际读出的数据流字节数
*/
uint32_t ebcMP2PNetWorkAPI::getReceiveDataStream(char *receiveDataStreamBuffer, uint32_t receiveDataStreamBufferSize)
{	
    std::string data;
    if(!engine.getUserDate(data))
        return 0;

    memcmp(receiveDataStreamBuffer, data.data(), data.size());
	return 0;
}

/*** end of file **************************************************************/
