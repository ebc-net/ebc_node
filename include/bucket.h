#ifndef BUCKET_H
#define BUCKET_H

#include <list>
#include <functional>
#include<bitset>
#include "nodeid.h"
#include "node.h"
#include "utils.h"

namespace NET
{
struct bucket {
    bucket() : cached() {}
    bucket(sa_family_t af, const NodeId& f = {}, time_point t = time_point::min()): af(af), first(f), cached() {}
    sa_family_t af {0};
    NodeId first {};
    std::list<Sp<Node>> nodes {};
    Sp<Node> cached;                    /* the address of a likely candidate */

    /** Return a random node in a bucket. */
    //Sp<Node> randomNode();

    //void sendCachedPing(net::NetworkEngine& ne);
    //void connectivityChanged() {
    //    time = time_point::min();
    //    for (auto& node : nodes)
    //        node->setTime(time_point::min());
    //}
};


class Bucket
{

public:
    using Kbucket = std::list<bucket>;
    using destoryNet = std::function<void(int)>;
    time_point grow_time {time_point::min()};
    Bucket(const NodeId &_id);
    NodeId middle(const Kbucket::const_iterator &it) const;
    Kbucket::iterator findBucket(const NodeId& id);
    std::vector<Sp<Node>> findClosestNodes(const NodeId& id, size_t count = MAX_NODE);
    NodeId randomId(const Kbucket::const_iterator &bucket);
    unsigned depth(const Kbucket::const_iterator& bucket) const;
    std::list<Sp<Node>> repNodes(const NodeId &id);
    inline bool contains(const Kbucket::const_iterator &it, const NodeId& id) const;
    bool onNewNode(const Sp<Node>& node, int confirm, bool isServer=false) ;
    bool findNode(const NodeId &id);
    bool bucketMaintenance(std::function<void(Sp<Node> &dstId, NodeId targetId)>,bool neighbour = true);//
    bool split(const Kbucket::iterator &b);
    void closeBucket(destoryNet d);

    void dump() const;
private:    
    Kbucket buckets;
    NodeId selfId;
    bool isEmpty() const;
};
}
#endif // BUCKET_H
