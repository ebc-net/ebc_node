#include "nodeid.h"
#include <iostream>
#include <cstring>
#include<QsLog.h>

namespace NET
{

NodeId::NodeId(const std::string &str)
{
    memcpy(data(), str.data(),ID_LENGTH);
}

std::string NodeId::toString()const
{
    std::string str;
    for(auto &i: *this)
        str.append(1,i);
    return str;
}

int NodeId::lowBit() const
{
    int i, j;
    for(i = ID_LENGTH-1; i >= 0; i--)
        if(at(i) != 0)
            break;
    if(i < 0)
        return -1;
    for(j = 7; j >= 0; j--)
        if((at(i) & (0x80 >> j)) != 0)
            break;
    return 8 * i + j;
}

int NodeId::xorCmp(const NodeId &_id1 , const NodeId &_id2)const
{
    for(unsigned i = 0; i < ID_LENGTH; i++) {
        uint8_t xor1, xor2;
        if(_id1[i] == _id2[i])
            continue;
        xor1 = _id1[i] ^ at(i);
        xor2 = _id2[i] ^ at(i);
        if(xor1 < xor2)
            return -1;
        else
            return 1;
    }
    return 0;
}

unsigned int NodeId::commBit( const NodeId& _id)const
{
    unsigned i, j;
    uint8_t x;
    for(i = 0; i < ID_LENGTH; i++) {
        if(at(i) != _id.at(i))
            break;
    }

    if(i == ID_LENGTH)
        return 8*ID_LENGTH;

    x = at(i) ^ _id.at(i);

    j = 0;
    while((x & 0x80) == 0) {
        x <<= 1;
        j++;
    }

    return 8 * i + j;
}

void NodeId::printNodeId(bool isExpire)const
{
    char tmp_id[2];
    std::string l_id;
    for(auto &i: *this)
    {
        sprintf(tmp_id,"%02x", i);
        l_id += tmp_id;
    }
    if(isExpire)
        QLOG_ERROR()<<l_id.c_str();
    else
        QLOG_INFO()<<l_id.c_str();
}

//void NodeId::setIdTitle() const
//{
//    char tmp_id[2];
//    std::string l_id;
//    for(auto &i: *this)
//    {
//        sprintf(tmp_id,"%02x", i);
//        l_id += tmp_id;
//    }
//    std::cout<<l_id.c_str()<<std::endl;
//}

bool NodeId::operator<(const NodeId &_id)
{
    return memcmp(_id.data(),data(),ID_LENGTH)>0;
}

bool NodeId::operator ==(const NodeId &_id)
{
    return memcmp(_id.data(),data(),ID_LENGTH) == 0;
}

}

