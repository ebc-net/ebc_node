/* create infomation  API/创建用户信息接口API函数:
   File Name        : ebcCreateInfomationAPI.h
   Version          : 1.00
   Date             : 2019-03-23
   Interface design : Wang Yueping(version 0.0.1)
   Software         : Wang Yueping(version 0.0.1)
   Language         : C++
   Development tool : Qt Creater 4.8.0
   Description      : create infomation API C++ header file

   (C) Copyright 2019  Right Chain(Shen Zhen) Technolegy Corp., LTD.
*/
#pragma once

#include <stdint.h>
#include "ebcCryptoLib.h"

//全记录节点交易输入数据结构
#define EBC_PRIVATE_KEY_SIZE      36
#define EBC_PUBLIC_KEY_SIZE       36
#define EBC_ADDRESS_SIZE          27
#define EBC_NODE_HASH_SIZE        32

//创建信息结构定义
typedef struct {
  uint8_t userPassword[8];                                      // 用户密码
  uint8_t userName[10];                                         // 用户名称
  uint8_t userMobile[12];                                       // 用户手机号码
  uint8_t userDevice[10];                                       // 用户手机设备
  uint8_t userMsgNotice;                                        // 用户消息通知
}NODE_CREATE_INFORMATION;

//用户账号信息结构定义
typedef struct {
  uint8_t nodePrivateKey[EBC_PRIVATE_KEY_SIZE];                 // 节点私钥
  uint8_t nodePublicKey[EBC_PUBLIC_KEY_SIZE];                   // 节点公钥
  uint8_t nodeAddress[EBC_ADDRESS_SIZE];                        // 节点地址
}NODE_ACCOUNT_INFOMATION;

class ebcCreateInfomationAPI : public ebcCryptoLib {

private:
  ebcCreateInfomationAPI();
  ~ebcCreateInfomationAPI();

public:
  /*** 私钥、公钥、地址内存保存于读取 ***/
  // （1）保存用户私钥、公钥、地址
  void putUserKeyPair(const uint8_t *privatekey, const uint8_t *publickey, const uint8_t *addddressID);

  // （2）读取用户私钥
  bool getUserPrivateKey(uint8_t *privateKey);

  // （3）读取用户公钥
  bool getUserPublicKey(uint8_t *publicKey);

  // （4）读取用户地址
  bool getUserAddress(uint8_t *addressID);

  /*** 用户创建信息保存与读取 ***/
  // （5）读取用户密码
  void getUserPassword(uint8_t *passWord) ;

  // （6）保存用户密码
  void putUserPassword(const uint8_t *passWord) ;

  // （7）读取创建信息
  void getUserCreateInfomation(NODE_CREATE_INFORMATION &userInfomation);

  // （8）保存创建信息
  void putUserCreateInfomation(const NODE_CREATE_INFORMATION &userInfomation);

  // （9）外存读取创建信息
  bool readUserCreateInfomation(NODE_CREATE_INFORMATION &userInfomation);

  // （10）外存保存创建信息
  bool writeUserCreateInfomation(const NODE_CREATE_INFORMATION &userInfomation);

  // （11）链上读取创建信息
  bool readUserCreateInfomationByChain(NODE_CREATE_INFORMATION &userInfomation);

  // （12）链上保存创建信息
  bool writeUserCreateInfomationToChain(const NODE_CREATE_INFORMATION &userInfomation);
};

/*** end of file **************************************************************/
