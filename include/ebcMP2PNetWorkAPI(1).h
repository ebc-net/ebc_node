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

#include <stdint.h>
#include "ebcCoreParameter.h"
#include "ebcCoreAPI.h"

// 定义网络数据结构
#define NETWORK_NODE_NUMBER  1500                                        // MP2P网络域最大组网节点数

typedef struct NETWORK_DOMAIN_TABLE {
	uint16_t onlineNodeNumber;                                           // 网络域在线节点数目
	uint8_t  onlineNodeAddress[NETWORK_NODE_NUMBER][EBC_ADDRESS_SIZE];   // 网络域在线节点地址
}NetworkDomainTable;

// MP2P网络与通讯模块接口函数类
class ebcMP2PNetWorkAPI {

private:

public:
	ebcMP2PNetWorkAPI();
	~ebcMP2PNetWorkAPI();

	// （1）创建网络函数
  bool createNetwork(const uint8_t *nodeAddress);
   
  // （2）刷新网络函数
  bool updateNetwork(const uint8_t *nodeAddress);

  // （3）读取网络表函数
  void getNetworkTable(const uint8_t *nodeAddress, NETWORK_DOMAIN_TABLE *networkTable);
    
  // （4）加入一个指定的节点到网络函数
  bool jionNodeToNetwork(const uint8_t *nodeAddress, const uint8_t *jionNodeAddress);
    
  // （5）断开网络中一个指定的节点函数
  bool breakNodeFormNetwork(const uint8_t *nodeAddress, const uint8_t *breakNodeAddress);
     
  // （6）由源地址发送数据流到目标地址函数
  bool sendDataStream(const uint8_t *sourceNodeAddress, const uint8_t *targetNodeAddress, const uint8_t *sendDataStreamBuffer, const uint32_t sendDataStreamBufferSize);
    
  // （7）从网络接收数据流中读取数据函数
  uint32_t getReceiveDataStream(uint8_t *receiveDataStreamBuffer, uint32_t receiveDataStreamBufferSize);
};

/*** end of file **************************************************************/
