#include "msgpack.h"
#include <assert.h>
#include "node.h"
#include<QsLog.h>


namespace NET {

msgPack::msgPack(const NET::NodeId &_id):self_id(_id)
{
    ebcMsg.Clear();
}

int msgPack::pack(config::MsgType type, void *msg, void *buf, int size,config::MsgSubType subType,const NET::NodeId dstId)
{
    ebcMsg.set_head(msgHead);//0xF5FA
    ebcMsg.set_version(NET_VERSION);//V0.0.0
    ebcMsg.set_src_id(&self_id, ID_LENGTH);
    ebcMsg.set_type(type);//EMPTY = 0,GET_NODE = 1,GET_DATA = 2,PING = 3,HEART = 4,REP = 5,PUNCH = 6,

    switch (type) {
    case config::MsgType::GET_NODE :
    {
        config::EbcNode* node = (config::EbcNode *)msg;  //发送get_node要将自己的NAT信息发过去
        config::EbcNodes* nodes = ebcMsg.mutable_nodes();//服务器发回节点的消息
        nodes->add_ebcnodes()->CopyFrom(*node);//??
        ebcMsg.set_length(nodes->ByteSizeLong());
        break;
    }
    case config::MsgType::REP:                       //服务器应答
    {
        ebcMsg.set_dst_id(&dstId, dstId.size());
        ebcMsg.set_sub_type(subType);
        if(config::MsgSubType::NODE == subType)
        {
            config::EbcNodes* nodes =  (config::EbcNodes*)msg;
            ebcMsg.mutable_nodes()->CopyFrom(*nodes);  //这个是深拷贝还是浅拷贝
            ebcMsg.set_length(nodes->ByteSizeLong());
        }
        else if(config::MsgSubType::DATA == subType)
        {
            std::string msg_body((char *)msg);
            ebcMsg.set_msg(msg_body);
            ebcMsg.set_length(msg_body.length());
        }
        break;
    }
    case config::MsgType::GET_DATA:
        break;
    case config::MsgType::PUNCH:
    {
        config::EbcNode* node = (config::EbcNode *)msg;  //发送需要打洞的对端节点的信息
        config::EbcNodes* nodes = ebcMsg.mutable_nodes();
        nodes->add_ebcnodes()->CopyFrom(*node);
        ebcMsg.set_length(nodes->ByteSizeLong());
        break;
    }
    default:
        break;
    }

    if(buf != nullptr)
        ebcMsg.SerializeToArray(buf, size);
    return ebcMsg.ByteSizeLong();
}

int msgPack::unpack(const void *buf, int len)
{
    assert(buf);//计算表达式buf,用 abort 来终止程序运行

    if(ebcMsg.ParseFromArray(buf, len) == false)//
    {
        QLOG_ERROR()<<"Parse msg error";
        return -1;
    }

    if((ebcMsg.head() != msgHead))
    {
        QLOG_ERROR()<<"msg head error";
        return -1;
    }

    if(ebcMsg.type() == config::MsgType::EMPTY)
    {
        QLOG_INFO()<<"get msg type empty";
        return -1;
    }

    if(ebcMsg.has_nodes())
    {
        if(ebcMsg.length() != ebcMsg.nodes().ByteSizeLong())
        {
            QLOG_ERROR()<<"get msg length error";
            return -1;
        }
    }
    else
    {
        if(ebcMsg.length() != ebcMsg.msg().length())
        {
            QLOG_ERROR()<<"get msg length error";
            return -1;
        }
    }

    //msgPrint();
    return 0;
}

void msgPack::msgPrint()
{
   QLOG_INFO()<<"version:"<<ebcMsg.version().c_str();
   QLOG_INFO()<<"src_id:";
   NodeId(ebcMsg.src_id()).printNodeId();

   switch (ebcMsg.type()) {
   case config::MsgType::GET_DATA:
           QLOG_INFO()<<"type :"<<"GET_DATA";
       break;
   case config::MsgType::GET_NODE:
           QLOG_INFO()<<"type :"<<"GET_NODE";
       break;
   case config::MsgType::REP:
           QLOG_INFO()<<"type :"<<"REP";
       break;
   case config::MsgType::PUNCH:
           QLOG_INFO()<<"type :"<<"PUNCH";
       break;
   case config::MsgType::EMPTY:
           QLOG_INFO()<<"type :"<<"EMPTY";
       break;
   default:
       break;
   }

   switch (ebcMsg.sub_type()) {
   case config::MsgSubType::DATA :
       QLOG_INFO()<<"sub_type :"<<"DATA";
       break;
   case config::MsgSubType::NODE :
       QLOG_INFO()<<"sub_type :"<<"NODE";
       break;
   case config::MsgSubType::EMPTY_SUB :
       QLOG_INFO()<<"sub_type :"<<"EMPTY";
       break;
   default:
       break;

   }

   QLOG_INFO()<<"length: "<<ebcMsg.length();

   if(ebcMsg.has_nodes())
   {
       config::EbcNodes nodes = ebcMsg.nodes();
       int node_count = nodes.ebcnodes_size();
       for(int i=0; i<node_count; ++i)
       {
           QLOG_INFO()<<"node id ";
           NodeId(nodes.ebcnodes(i).id()).printNodeId();
           QLOG_INFO()<<"node ip "<<nodes.ebcnodes(i).ip();//注意，无法打印
           QLOG_INFO()<<"node port "<<nodes.ebcnodes(i).port_nat();
       }
   }
   else if(!ebcMsg.msg().empty())
   {
       QLOG_INFO()<<"msg body: "<<ebcMsg.msg().c_str();
   }
}

}
