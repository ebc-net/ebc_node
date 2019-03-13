#ifndef NODEID_H
#define NODEID_H

#include"utils.h"
#include<array>
namespace NET{
class NodeId : public std::array<uint8_t, ID_LENGTH>
{
public:
    NodeId(const std::string&);
    NodeId(){}
    std::string toString()const;
    int lowBit()const;
    int xorCmp( const NodeId& _id1, const NodeId& _id2)const;
    unsigned int commBit(const NodeId&)const;
    void printNodeId()const;
    bool operator <  (const NodeId & _id);
    bool operator == (const NodeId & _id);

};
}
#endif // NODEID_H
