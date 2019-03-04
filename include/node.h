#ifndef NODE_H
#define NODE_H

#include <vector>

#include "udt.h"
#include "sockaddr.h"
#include "utils.h"

namespace NET{
class Node
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
        FULL_CLONE = 0, //全克隆
        RESTRICT , //限制型
        PORT_RESTRICT , //端口限制
        SYMMTRIC,   //对称型
    }NatType;


    /*******************************function*****************/
    Node():id({0}),state(DISCONNECTED), nat(SYMMTRIC), ping_count(0) {}
    Node(const NodeId & _id, const SockAddr _addr=nullptr, const NatType _nat=SYMMTRIC, const NodeState  _state=DISCONNECTED);

    const NodeId& getId();

    void setAddr(const SockAddr &_addr);
    const SockAddr& getAddr();

    void setState(const NodeState &_state);
    const NodeState& getState();

    void setNat(const NatType &_nat);
    const NatType& getNatType();

    void setSock(const UDTSOCKET& _sock);
    const UDTSOCKET& getSock();

    void setPingCount(const uint8_t &count);
    const uint8_t &getPingCount();

    bool operator==(const Node& o);
    Node& operator=(const Node& o);


    /**************static function******************/
    static void printNodeId(const NodeId &);
    static void printNodeId(const std::string& );
    static void printNodeState(const NodeState _state);
    static void printNatType(const NatType _nat);
    static void NodeId2String(const NodeId&, std::string &);
    static void String2NodeId(const std::string&, NodeId&);
    static void printNode(Node &);
    static int lowBit(const NodeId &);
    static int idCmd(const NodeId& _id1, const NodeId& _id2);
    static int xorCmp(const NodeId& _id, const NodeId& _id1, const NodeId& _id2);

    inline int idCmp(const NodeId&);
    inline unsigned int commBit(const NodeId&);
    inline void setExpired() {state = NodeState::DISCONNECTED;};//失效
    inline bool isExpired() { return state == Node::DISCONNECTED;}

private:
    NodeId id;
    SockAddr addr;
    NodeState state;
    NatType nat;
    time_point last_tm;  //上次消息达到的时间
    uint8_t  ping_count;
    UDTSOCKET sock;
};
}
#endif // NODE_H
