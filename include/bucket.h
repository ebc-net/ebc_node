#ifndef BUCKET_H
#define BUCKET_H

#include <list>

#include "node.h"
#include "utils.h"

namespace NET
{
class Bucket
{
public:

    Bucket();

    static constexpr uint8_t maxNode {16};

    NodeId firstId;
    std::list<Sp<Node>> nodes;
};

}
#endif // BUCKET_H
