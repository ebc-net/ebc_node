/* MP2P communication and network interface API/MP2P通讯与网络接口API函数
   File Name        : ebcMP2PNetWorkAPI.h
   Version          : 0.0.0
   Date             : 2019-03-23
   Interface design : Wang Yueping(version 0.0.1)
   Software         : Xu Xiaona,He Xiaoen,Sun jiarui(version 0.0.1)
   Language         : C++
   Development tool : Qt Creater 4.8.0
   Description      :MP2P communication and network interface API C++ header file

   (C) Copyright 2019  Right Chain(Shen zhen) Technolegy Corp., LTD.
*/
#pragma once

#include<memory>
#include <stdint.h>
#include "ebcCoreParameter.h"
#include"netengine.h"
#include "QsLog.h"

// 定义网络数据结构A
#define NETWORK_NODE_NUMBER  1500                                        // MP2P网络域最大组网节点数

typedef struct NETWORK_DOMAIN_TABLE
{
    uint16_t onlineNodeNumber;                                           // 网络域在线节点数目
    std::list<std::string> onlineNodeAddress;   // 网络域在线节点地址
}NetworkDomainTable;

// MP2P网络与通讯模块接口函数类
class ebcMP2PNetWorkAPI
{

private:
NET::NetEngine engine;
public:
    ebcMP2PNetWorkAPI();
    ~ebcMP2PNetWorkAPI();
	// （1）创建网络函数
  bool createNetwork(const char *nodeAddress);

  // （1）创建网络函数
void closeNetwork();

  // （2）刷新网络函数
  bool updateNetwork();

  // （3）读取网络表函数
  void getNetworkTable(NETWORK_DOMAIN_TABLE *networkTable);
    
  // （4）加入一个指定的节点到网络函数
  bool joinNodeToNetwork(const char *joinNodeAddress);
    
  // （5）断开网络中一个指定的节点函数
  bool breakNodeFormNetwork(const char *breakNodeAddress);
     
  // （6）由源地址发送数据流到目标地址函数
  bool sendDataStream(const char *sourceNodeAddress, const char *targetNodeAddress, const char *sendDataStreamBuffer, const uint32_t sendDataStreamBufferSize);
    
  // （7）从网络接收数据流中读取数据函数
  uint32_t getReceiveDataStream(char *receiveDataStreamBuffer, uint32_t receiveDataStreamBufferSize);
  // （9）读取接收数据包消息函数
  uint16_t getReceiveDataPackageMessage();
};

/*** end of file **************************************************************/
