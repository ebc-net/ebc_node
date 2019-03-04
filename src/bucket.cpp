#include "bucket.h"
#include <algorithm>
#include "QsLog.h"

namespace NET
{

Bucket::Bucket(const NodeId &_id):selfId(_id)
{
    buckets.push_front(bucket(AF_INET));
}

NodeId Bucket::middle(const Kbucket::const_iterator &it) const
{
    int bit1 = Node::lowBit(it->first);
    int bit2 = std::next(it)==buckets.end() ? -1:Node::lowBit(std::next(it)->first);

    int bit = std::max(bit1, bit2) + 1;
    if(bit > 8*ID_LENGTH)
        throw std::out_of_range("End of table");

    NodeId newId = it->first;
    newId.at(bit/8) |= (0x80)>>(bit%8);

    return newId;
}

std::vector<Sp<Node> > Bucket::findClosestNodes(const NodeId &id, size_t count)
{
    std::vector<Sp<Node>> nodes;
    nodes.reserve(count);
    auto bucket = findBucket(id);

    if (bucket == buckets.end()) { return nodes; }

    auto sortedBucketInsert = [&](const struct bucket &b) {
        for (auto n : b.nodes) {
            if (not n->isExpired())
                continue;

            auto here = std::find_if(nodes.begin(), nodes.end(),
                                     [&id,&n](Sp<Node> &node) {
                return Node::xorCmp(id, n->getId(), node->getId()) < 0;
            }
            );
            nodes.insert(here, n);
        }
    };

    auto itn = bucket;
    auto itp = (bucket == buckets.begin()) ? buckets.end() : std::prev(bucket);
    while (nodes.size() < count && (itn != buckets.end() || itp != buckets.end())) {
        if (itn != buckets.end()) {
            sortedBucketInsert(*itn);
            itn = std::next(itn);
        }
        if (itp != buckets.end()) {
            sortedBucketInsert(*itp);
            itp = (itp == buckets.begin()) ? buckets.end() : std::prev(itp);
        }
    }

    // shrink to the count closest nodes.
    if (nodes.size() > count) {
        nodes.resize(count);
    }
    return nodes;
}

Bucket::Kbucket::iterator Bucket::findBucket(const NodeId &id)
{
    if(isEmpty())
        return buckets.end();

    auto it = buckets.begin(); //这里为什么不能把it定义为引用?
    while(it != buckets.end())
    {
        if(std::next(it) == buckets.end())
            return  it;
        if(Node::idCmd(id, std::next(it)->first) < 0)
            return it;
		it = std::next(it);
    }
}

unsigned Bucket::depth(const Kbucket::const_iterator& it) const
{
	if (it == buckets.end())
	  return 0;
	int bit1 = Node::lowBit(it->first);
	int bit2 = std::next(it) != buckets.end() ? Node::lowBit(std::next(it)->first) : -1;
	return std::max(bit1, bit2)+1;
}

bool Bucket::contains(const Kbucket::const_iterator &it, const NodeId &id) const//判断是否在bucket中
{
    return Node::idCmd(it->first, id) <= 0
            && (std::next(it) == buckets.end()  || Node::idCmd(id, std::next(it)->first) < 0);
}

bool Bucket::onNewNode(const Sp<Node>& node, int confirm)
{
    auto b = findBucket(node->getId());
    if (b == buckets.end())
        return false;

    if (confirm == 2)
        b->time = std::chrono::steady_clock::now();

    for (auto& n : b->nodes) {
        if (n == node)
            return false;
    }

    bool mybucket = contains(b, selfId);
//    if (mybucket) {
//        //grow_time = now;
//    }

    if (b->nodes.size() >= MAX_NODE) {//大于8个开始分桶
		//QLOG_WARN()<<mybucket;
//        if (mybucket ) {
        if (mybucket || depth(b) < 6) {
			//QLOG_WARN()<<"split !!";
            split(b);
            return onNewNode(node, confirm);
        }

        /* No space for this node.  Cache it away for later. */
        if (confirm or not b->cached)
            b->cached = node;
    } else {
        /* Create a new node. */
        b->nodes.emplace_front(node);

    }
    return true;
}

bool Bucket::findNode(const NodeId &id)
{
    auto it = findBucket(id);
    for (auto & n : it->nodes)
    {
        if(Node::idCmd(id,n->getId()) == 0 )
            return true;
    }
    return false;
}
//服务器还给客户的ID表
std::list<Sp<Node> > Bucket::repNodes(const NodeId &id)//服务器REP节点的96个节点
{
    std::list<Sp<Node>> nodes;
    auto it = findBucket(id);
    int bit = depth(it) - 1;
    auto tmp = it->nodes;
    nodes.splice(nodes.end(),tmp);
    NodeId id_first = it->first;

//    QLOG_WARN()<<"bit is : "<<bit;
//    QLOG_WARN()<<"first node id is : ";
//    Node::printNodeId(id_first);
//    QLOG_WARN()<<"REP NODES number is : "<<nodes.size();


    for( ;bit >= 0; bit--)
    {
        int i = bit/8;
        int j = bit%8;
        id_first[i] ^= (0x80 >> j);//closest
        auto it = findBucket(id_first);
        auto tmp = it->nodes;
        nodes.splice(nodes.end(),tmp);

//        QLOG_WARN()<<"first node id is : ";
//        Node::printNodeId(id_first);


        id_first[i] &= (0xfe <<(7 - j));

//        QLOG_WARN()<<"REP NODES number is : "<<nodes.size();
    }

    return nodes;
}

bool Bucket::onNewNodesrv(const Sp<Node>& node, int confirm)//服务器端分桶
{
   auto b = findBucket(node->getId());//查找节点属于哪个桶
    if (b == buckets.end())
        return false;

    if (confirm == 2)
        b->time = std::chrono::steady_clock::now();

    for (auto& n : b->nodes) {//查找桶内有无此节点
        if (n == node)
            return false;
    }
    bool mybucket = contains(b, selfId);//本地节点是否属于b桶
    if (mybucket) {
        //grow_time = now;
    }

    if (b->nodes.size() >= MAX_NODE) {//如果大于8个节点就分桶
        if (mybucket || depth(b) < 11) {  //深度11层
            split(b);
            return onNewNodesrv(node, confirm);
        }
        /* No space for this node.  Cache it away for later. */
        if (confirm or not b->cached)
            b->cached = node;
    } else {
        /* Create a new node. */
        b->nodes.emplace_front(node);

    }
    return true;
}
bool Bucket::split(const Kbucket::iterator &b)
{
    NodeId new_first_id;
    try {
        new_first_id = middle(b);
    } catch (const std::out_of_range& e) {
        return false;
    }

	QLOG_WARN()<<"new bucket : ";
    Node::printNodeId(new_first_id);
    // Insert new bucket
    buckets.insert(std::next(b), bucket {b->af, new_first_id, b->time});//从next(b)之前开始插入，

    // Re-assign nodes重新分派节点
    std::list<Sp<Node>> nodes {};//std::shared_ptr<T>;
    nodes.splice(nodes.begin(), b->nodes);
    while (!nodes.empty()) {
        auto n = nodes.begin();
        auto b = findBucket((*n)->getId());
        if (b == buckets.end())
            nodes.erase(n);
        else
            b->nodes.splice(b->nodes.begin(), nodes, n);
    }
    return true;
}

void Bucket::closeBucket(Bucket::destoryNet d)
{
    for(auto& b: buckets)
    {
        for(auto& n: b.nodes)
        {
            d(n->getSock());
        }
    }
}



bool Bucket::isEmpty() const
{
    return buckets.empty();
}

void Bucket::dump() const
{
	for(auto& b: buckets)
	{
		QLOG_WARN()<<"bucket size= "<<b.nodes.size();
		Node::printNodeId(b.first);
		for(auto& n: b.nodes)
		{
			Node::printNodeId(n->getId());
		}
	}
	QLOG_WARN()<<"k-bucket num = "<<buckets.size();
}

}
