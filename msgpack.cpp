#include "msgpack.h"
#include <assert.h>
#include "node.h"


namespace NET {

msgPack::msgPack(const NET::NodeId &_id):self_id(_id)
{
    ebcMsg.Clear();
}

int msgPack::pack(config::MsgType type, void *msg, void *buf, int size,config::MsgSubType subType,const NET::NodeId dstId)
{
    ebcMsg.set_head(msgHead);
    ebcMsg.set_version(NET_VERSION);
    ebcMsg.set_src_id(&self_id, ID_LENGTH);
    ebcMsg.set_type(type);

    switch (type) {
    case config::MsgType::GET_NODE :
    {
        config::EbcNode* node = (config::EbcNode *)msg;  //发送get_node要将自己的NAT信息发过去
        config::EbcNodes* nodes = ebcMsg.mutable_nodes();
        nodes->add_ebcnodes()->CopyFrom(*node);
        ebcMsg.set_length(nodes->ByteSizeLong());
        break;
    }
    case config::MsgType::REP:
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
    assert(buf);

    if(ebcMsg.ParseFromArray(buf, len) == false)
    {
        std::cout<<"Parse msg error"<<std::endl;
        return -1;
    }

    if((ebcMsg.head() != msgHead))
    {
        std::cout<<"msg head error"<<std::endl;
        return -1;
    }

    if(ebcMsg.type() == config::MsgType::EMPTY)
    {
        std::cout<<"get msg type empty"<<std::endl;
        return -1;
    }

    if(ebcMsg.has_nodes())
    {
        if(ebcMsg.length() != ebcMsg.nodes().ByteSizeLong())
        {
            std::cout<<"get msg length error"<<std::endl;
            return -1;
        }
    }
    else
    {
        if(ebcMsg.length() != ebcMsg.msg().length())
        {
            std::cout<<"get msg length error"<<std::endl;
            return -1;
        }
    }

	msgPrint();
    return 0;
}

void msgPack::msgPrint()
{
   std::cout<<"version:"<<ebcMsg.version()<<std::endl;
   std::cout<<"src_id:";
   Node::printNodeId(ebcMsg.src_id());
   std::cout<<"dst_id:";  //注意，无法打印
   Node::printNodeId(ebcMsg.dst_id());

   switch (ebcMsg.type()) {
   case config::MsgType::GET_DATA:
           std::cout<<"type :"<<"GET_DATA"<<std::endl;
       break;
   case config::MsgType::GET_NODE:
           std::cout<<"type :"<<"GET_NODE"<<std::endl;
       break;
   case config::MsgType::REP:
           std::cout<<"type :"<<"REP"<<std::endl;
       break;
   case config::MsgType::PUNCH:
           std::cout<<"type :"<<"PUNCH"<<std::endl;
       break;
   case config::MsgType::EMPTY:
           std::cout<<"type :"<<"EMPTY"<<std::endl;
       break;
   default:
       break;
   }

   switch (ebcMsg.sub_type()) {
   case config::MsgSubType::DATA :
       std::cout<<"sub_type :"<<"DATA"<<std::endl;
       break;
   case config::MsgSubType::NODE :
       std::cout<<"sub_type :"<<"NODE"<<std::endl;
       break;
   case config::MsgSubType::EMPTY_SUB :
       std::cout<<"sub_type :"<<"EMPTY"<<std::endl;
       break;
   default:
       break;

   }

   std::cout<<"length: "<<ebcMsg.length()<<std::endl;

   if(ebcMsg.has_nodes())
   {
       config::EbcNodes nodes = ebcMsg.nodes();
       int node_count = nodes.ebcnodes_size();
       for(int i=0; i<node_count; ++i)
       {
           std::cout<<"node id ";
           Node::printNodeId(nodes.ebcnodes(i).id());
           std::cout<<"node ip "<<nodes.ebcnodes(i).ip()<<std::endl;//注意，无法打印
           std::cout<<"node port "<<nodes.ebcnodes(i).port_nat()<<std::endl;
       }
   }
   else if(!ebcMsg.msg().empty())
   {
       std::cout<<"msg body: "<<ebcMsg.msg()<<std::endl;
   }
}

}
