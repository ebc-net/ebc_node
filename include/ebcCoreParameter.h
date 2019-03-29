/* core API common paramaters/核心API公用参数:
   File Name        : ebcCoreAPI.h
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

//定义参数
#define EBC_PRIVATE_KEY_SIZE      36
#define EBC_PUBLIC_KEY_SIZE       36
#define EBC_ADDRESS_SIZE          27
#define EBC_NODE_HASH_SIZE        32

//定义命令
enum {
  EBC_CMD_UPLOAD = 0,           // 分布式上传文件命令
  EBC_CMD_DOWNLOAD = 1,         // 分布式下载文件命令
};

/*** end of file **************************************************************/
