#include "node.h"
#include <iostream>
#include <cstring>


namespace NET
{
Node::Node(const NodeId & _id, const SockAddr _addr, const NatType _nat, const NodeState  _state)
    :addr(_addr), state(_state),nat(_nat),last_tm(time_point::max()), ping_count(0)
{
    if(_id.size() > ID_LENGTH)
    {
        std::cout<<"id length "<<_id.size()<<"error"<<std::endl;
        return ;
    }

    id = _id;
}

const NodeId &Node::getId()
{
    return id;
}

void Node::setAddr(const SockAddr &_addr)
{
    addr = _addr;
}

const SockAddr &Node::getAddr()
{
    return addr;
}

void Node::setState(const Node::NodeState &_state)
{
    state = _state;
}

const Node::NodeState &Node::getState()
{
    return state;
}

void Node::setNat(const Node::NatType &_nat)
{
    nat = _nat;
}

const Node::NatType &Node::getNatType()
{
    return nat;
}

void Node::setSock(const UDTSOCKET &_sock)
{
    sock = _sock;
}

const UDTSOCKET &Node::getSock()
{
   return sock;
}

void Node::setLastTm(const Node::time_point &_tm)
{
    last_tm = _tm;
}

const Node::time_point &Node::getLastTime()
{
    return last_tm;
}

bool Node::isExpired(const Node::time_point &now)
{
#if 0
   if(now - last_tm >= EXPIRE_TM)
       return true;
#endif

   return false;
}

void Node::setPingCount(const uint8_t &count)
{
    ping_count = count;
}

const uint8_t &Node::getPingCount()
{
    return ping_count;
}

bool Node::operator==(const Node &o)
{
    return memcmp(this, &o, sizeof(*this)) == 0;  //可能用BUG， 因为sockaddr是指针
}

Node &Node::operator=(const Node &o)
{
   id = o.id;
   addr = o.addr;
   state = o.state;
   nat = o.nat;
   last_tm = o.last_tm;
   ping_count = o.ping_count;
   sock = o.sock;

   return *this;
}

void Node::printNodeId(const NodeId &_id)
{
   for(auto &i: _id)
       printf("%02x", i);
   printf("\n");
}

void Node::printNodeId(const std::string &_id)
{
   NodeId nid;
   String2NodeId(_id, nid);
   printNodeId(nid);
}

void Node::printNodeState(const Node::NodeState _state)
{
    switch (_state) {
    case CONNECTED:
        std::cout<<"connected"<<std::endl;
        break;
    case PAUNCH:
        std::cout<<"PAUNCH"<<std::endl;
        break;
    case DISCONNECTED:
        std::cout<<"DISCONNECTED"<<std::endl;
        break;
    default:
        break;
    }
}

void Node::printNatType(const Node::NatType _nat)
{
    switch (_nat) {
    case FULL_CLONE:
        std::cout<<"FULL CLONE"<<std::endl;
        break;
    case RESTRICT:
        std::cout<<"RESTRICT"<<std::endl;
        break;
    case PORT_RESTRICT:
        std::cout<<"PORT_RESTRICT"<<std::endl;
        break;
    case SYMMTRIC:
        std::cout<<"SYMMTRIC"<<std::endl;
        break;
    default:
        break;
    }
}

void Node::NodeId2String(const NodeId &_id, std::string &_str)
{
    _str.clear();
    _str.resize(ID_LENGTH);
    for(auto &i:_id)
        _str.append(1, i);
}

void Node::String2NodeId(const std::string &_str, NodeId &_id)
{
    memcpy(_id.data(), _str.data(), ID_LENGTH);
}

void Node::printNode(Node& _node)
{
    printNodeId(_node.getId());
    print_addr(_node.getAddr());
    printNodeState(_node.getState());
    printNatType(_node.getNatType());
}

}


