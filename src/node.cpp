#include "node.h"
#include <iostream>
#include <cstring>
#include<QsLog.h>


namespace NET
{

Node::Node(const NodeId & _id, const SockAddr _addr, const NatType _nat, const NodeState  _state)
    :id(_id),addr(_addr), state(_state),nat(_nat),last_tm(time_point::max()), ping_count(0)
{
    if(_id.size() > ID_LENGTH)
    {
        QLOG_ERROR()<<"id length "<<_id.size()<<"error";
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

void Node::printNodeState(const Node::NodeState _state)
{
    switch (_state) {
    case CONNECTED:
        QLOG_INFO()<<"connected";
        break;
    case PAUNCH:
        QLOG_INFO()<<"PAUNCH";
        break;
    case DISCONNECTED:
        QLOG_INFO()<<"DISCONNECTED";
        break;
    default:
        break;
    }
}

void Node::printNatType(const Node::NatType _nat)
{
    switch (_nat) {
    case FULL_CLONE:
        QLOG_INFO()<<"FULL CLONE";
        break;
    case RESTRICT:
        QLOG_INFO()<<"RESTRICT";
        break;
    case PORT_RESTRICT:
        QLOG_INFO()<<"PORT_RESTRICT";
        break;
    case SYMMTRIC:
        QLOG_INFO()<<"SYMMTRIC";
        break;
    default:
        break;
    }
}

void Node::printNode(Node& _node)
{
    _node.id.printNodeId();
    print_addr(_node.getAddr());
    printNodeState(_node.getState());
    printNatType(_node.getNatType());
}

}
