#ifndef BUCKET_H
#define BUCKET_H

#include <list>
#include <functional>
#include<bitset>
#include<random>
#include "nodeid.h"
#include "node.h"
#include "utils.h"
#include<ctime>

namespace NET
{
struct bucket
{
    bucket() : cached() {}
    bucket(sa_family_t af, const NodeId& f = {}): af(af), first(f), cached() {}
    sa_family_t af {0};
    NodeId first {};
    std::list<Sp<Node>> nodes {};
    time_point lastTime = time_point::min();
    Sp<Node> cached;                    /* the address of a likely candidate */
};


class Bucket
{

public:
    using Kbucket = std::list<bucket>;
    using destoryNet = std::function<void(int)>;
    using sendNode = std::function<void(Sp<Node> &dstId, NodeId targetId)>;
    time_point grow_time {time_point::min()};
    Bucket(const NodeId &_id);
    NodeId middle(const Kbucket::const_iterator &it) const;
    Kbucket::iterator findBucket(const NodeId& id);
    std::vector<Sp<Node>> findClosestNodes(const NodeId& id, size_t count = MAX_NODE);
    NodeId randomId(const Kbucket::const_iterator &bucket);
    unsigned depth(const Kbucket::const_iterator& bucket) const;
    int xorDepth(const NodeId &);
    std::list<Sp<Node>> repNodes(const NodeId &id);
    std::list<Sp<Node>> broadcastOthers(const NodeId &);
    std::list<Sp<Node>> broadcastLocal();
    inline bool contains(const Kbucket::const_iterator &it, const NodeId& id) const;
    bool onNewNode(const Sp<Node>& node, int confirm, bool isServer=false) ;
    bool findNode(const NodeId &id);
    Sp<Node> getNode(const NodeId &id);
    bool bucketMaintenance(sendNode,bool neighbour = true);//用neighbour = true 扩桶，neighbour = false 桶维护
    bool split(const Kbucket::iterator &b);
    void closeBucket(destoryNet d);
    void delExpNode();
    void delEmpBuk();
    void dump(int type = 0) const;
    bool bucketIsEmpty(NodeId id); //have no nodes;
    NodeId getSelfId();
    Kbucket buckets;

private:    

    NodeId selfId;
    bool isEmpty() const;

};
}
#endif // BUCKET_H
