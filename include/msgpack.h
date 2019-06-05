#ifndef MSGPACK_H
#define MSGPACK_H

#include"nodeid.h"
#include "utils.h"
#include "msg.pb.h"
#include <string>
#include <set>

namespace NET {

class msgPack
{
public:
    msgPack(const NET::NodeId& _id);
    int pack(config::MsgType type, int aport = 0,const void *msg=nullptr, void* buf=nullptr, int size=0, config::MsgSubType subType = config::MsgSubType::EMPTY_SUB,int msgSize = 0,NodeId dstId ={0});
    int unpack(const void *buf, int len);
    void msgPrint();
    config::EbcMsg ebcMsg;

private:
    const NET::NodeId self_id;
    static constexpr int msgHead {0xF5FA};

};
}

#endif // MSGPACK_H
