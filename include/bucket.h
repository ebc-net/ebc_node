#ifndef BUCKET_H
#define BUCKET_H

#include <list>
#include <functional>

#include "node.h"
#include "utils.h"

namespace NET
{
struct bucket {
    bucket() : cached() {}
    bucket(sa_family_t af, const NodeId& f = {}, time_point t = time_point::min())
        : af(af), first(f), time(t), cached() {}
    sa_family_t af {0};
    NodeId first {};
    time_point time {time_point::min()}; /* time of last reply in this bucket */
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

   Bucket(const NodeId &_id);

   NodeId middle(const Kbucket::const_iterator &it) const;
   std::vector<Sp<Node>> findClosestNodes(const NodeId& id, time_point now, size_t count = 8) const;
   Kbucket::iterator findBucket(const NodeId& id);
   unsigned depth(const Kbucket::const_iterator& bucket) const;
   std::list<Sp<Node>> repNodes(const NodeId &id);

   inline bool contains(const Kbucket::const_iterator &it, const NodeId& id) const;

   bool onNewNode(const Sp<Node>& node, int confirm) ;
   bool findNode(const NodeId &id);

   bool onNewNodesrv(const Sp<Node>& node, int confirm) ;
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
