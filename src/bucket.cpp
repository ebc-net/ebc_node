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

NodeId Bucket::randomId(const Kbucket::const_iterator &b)
{
    int bit1 = Node::lowBit(b->first);
    int bit2 = std::next(b)==buckets.end() ? -1:Node::lowBit(std::next(b)->first);

    int bit = std::max(bit1, bit2) + 1;
    if(bit > 8*ID_LENGTH)
        return b->first;
    int bt = bit / 8;
    NodeId id_return;
    std::copy_n(b->first.cbegin(),bt, id_return.begin());
    id_return[bt] = b->first[bt] & (0xFF00 >> (bit % 8));
    id_return[bt] |= 0xff&rand()%8 >> (bit % 8);
    for (unsigned i = bt + 1; i < ID_LENGTH; i++)
        id_return[i] = 0xff&rand()%8;
    return id_return;
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

bool Bucket::onNewNode(const Sp<Node>& node, int confirm, bool isServer)
{
    auto b = findBucket(node->getId());
    if (b == buckets.end())
        return false;

    if (confirm == 2)
        b->time = clock::now();

    for (auto& n : b->nodes) {
        if (n == node)
            return false;
    }

    bool mybucket = isServer?false:contains(b, selfId);
    if(mybucket)
    {
        grow_time = clock::now();
    }
    if (b->nodes.size() >= MAX_NODE)
    {//大于8个开始分桶
        //QLOG_WARN()<<mybucket;
        int dNum = isServer?11:6;
        if (mybucket || depth(b) < dNum) {
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

#if 0
Bucket::Kbucket::iterator Bucket::neighbourhoodMaintenance()
{
    auto b = findBucket(selfId);
    return b;
    
}
#endif

bool Bucket::bucketMaintenance(std::function<void(Sp<Node> &dstNode, NodeId targetId)> sendFindNode, bool neighbour)
{
    int goodNode = 0;
    Kbucket::iterator q;
    if(!neighbour)
        q = buckets.begin();
    else
        q=findBucket(selfId);
    Sp<Node> dstNode{};
    while ( q != buckets.end())
    {
        if(q->nodes.empty())
        {
            q = std::next(q);
            continue;
        }

        for(auto &n : q->nodes)
        {
            if(!n->isExpired())
            {
                goodNode++;
                dstNode = n;
            }
        }

        if(!neighbour && goodNode < 3 && goodNode >0 )
        {
            sendFindNode(dstNode, randomId(q));
            return true;
        }
        if(neighbour && goodNode >0)
        {

                    sendFindNode(dstNode, selfId);
                       return true;
        }

        //在最邻近的桶找一个活节点，给他发送findClosestNode（一个需要维护桶范围内的随机ID），以下代码实现的就是这个功能
        //写的不是很容易看懂，可以跳过不看
        else if (goodNode == 0 )
        {
            auto doNetFunc = [&](struct bucket& b)
            {
                auto it = std::find_if(b.nodes.begin(), b.nodes.end(), [&](Sp<Node>& node)
                {
                        if(!node->isExpired())
                {
                        if(!neighbour)
                        sendFindNode(node, randomId(q));
                        else
                        sendFindNode(node, selfId);
                        return true;
            }
                        return false;
            }
                        );

                if(it != b.nodes.end())
                    return true;

                return false;
            };

            auto itn = q;
            auto itp = (q == buckets.begin()) ? buckets.end() : std::prev(q);
            while ((itn != buckets.end() || itp != buckets.end()))
            {
                if (itn != buckets.end())
                {
                    if(doNetFunc(*itn))
                        return true;
                    itn = std::next(itn);
                }
                if (itp != buckets.end())
                {
                    if(doNetFunc(*itp))
                        return true;
                    itp = (itp == buckets.begin()) ? buckets.end() : std::prev(itp);
                }
            }

            return false;
        }
        if(!neighbour)
            q = std::next(q);
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
