#ifndef NODE_H
#define NODE_H

#include <vector>

#include "udt.h"
#include "sockaddr.h"
#include "utils.h"
#include"nodeid.h"
namespace NET{
class   Node
{
public:
#define MAX_PING  (3)

    //static constexpr std::chrono::minutes EXPIRE_TM {1};  //????

    typedef enum _NodeState
    {
        STABLE = 0,    //节点以连接并且稳定
        CONNECTED ,    //节点已连接，但是不稳定
        PAUNCH ,       //出于NAT穿透状态
        DISCONNECTED,  //连接已失败
    }NodeState;

    typedef enum _NatType
    {
        EMPTY = 0,
        FULL_CLONE , //全克隆
        RESTRICT , //限制型
        PORT_RESTRICT , //端口限制
        SYMMTRIC,   //对称型
    }NatType;


    /*******************************function*****************/
    Node():id(), state(CONNECTED), nat(EMPTY){}
    Node(const NodeId & _id, const SockAddr _addr=nullptr, const NatType _nat= EMPTY, const NodeState  _state=CONNECTED);

    const NodeId& getId();

    void setAddr(const SockAddr &_addr);
    const SockAddr& getAddr();

    void setState(const NodeState &_state);
    const NodeState& getState();

    void setNat(const NatType &_nat);
    const NatType& getNatType();

    void setSock(const UDTSOCKET& _sock);
    const UDTSOCKET& getSock();

    bool operator==(const Node& o);
    Node& operator=(const Node& o);

    /**************static function******************/
    static void printNodeState(const NodeState _state);
    static void printNatType(const NatType _nat);
    static void printNode(Node &);
    inline void setExpired() {state = NodeState::DISCONNECTED;}//失效
    inline bool isExpired() { return state == Node::DISCONNECTED;}

private:
    NodeId id;
    SockAddr addr;
    NodeState state;
    NatType nat;
    UDTSOCKET sock;

};
}
#endif // NODE_H
