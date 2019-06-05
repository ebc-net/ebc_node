#include "bucket.h"
#include <algorithm>
#include "QsLog.h"
#include<random>
#include<ctime>



namespace NET
{
static std::mt19937 rd(std::chrono::system_clock::now().time_since_epoch().count());
#ifdef _WIN32
static std::uniform_int_distribution<int> rand_byte{ 0, std::numeric_limits<uint8_t>::max() };
#else
static std::uniform_int_distribution<uint8_t> rand_byte;
#endif

Bucket::Bucket(const NodeId &_id):selfId(_id)
{
    buckets.push_front(bucket(AF_INET));
}

NodeId Bucket::middle(const Kbucket::const_iterator &it) const
{
    int bit1 = it->first.lowBit();
    int bit2 = std::next(it)==buckets.end() ? -1:std::next(it)->first.lowBit();

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
            if ( n->isExpired())
                continue;

            auto here = std::find_if(nodes.begin(), nodes.end(),
                                     [&id,&n](Sp<Node> &node) {
                return id.xorCmp(n->getId(), node->getId()) < 0;
            });
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

    auto it = buckets.begin();
    while(it != buckets.end())
    {
        if(std::next(it) == buckets.end())
            return  it;
        if(id < std::next(it)->first)
            return it;
        it = std::next(it);
    }
}

NodeId Bucket::randomId(const Kbucket::const_iterator &b)
{
    int bit1 = b->first.lowBit();
    int bit2 = std::next(b)==buckets.end() ? -1:std::next(b)->first.lowBit();
    int bit = std::max(bit1, bit2) + 1;
    if(bit > 8*ID_LENGTH)
        return b->first;
    int bt = bit / 8;
    NodeId id_return;
    std::copy_n(b->first.cbegin(),bt, id_return.begin());
    id_return[bt] = b->first[bt] & (0xFF00 >> (bit % 8));
    id_return[bt] |=  rand_byte(rd) >> (bit % 8);
    for (unsigned i = bt + 1; i < ID_LENGTH; i++)
        id_return[i] = rand_byte(rd);
    return id_return;
}

unsigned Bucket::depth(const Kbucket::const_iterator& it) const
{
    if (it == buckets.end())
        return 0;
    int bit1 = it->first.lowBit();
    int bit2 = std::next(it) != buckets.end() ? std::next(it)->first.lowBit() : -1;
    return std::max(bit1, bit2)+1;
}

//判断是否在bucket中
bool Bucket::contains(const Kbucket::const_iterator &it, const NodeId &id) const
{
    return (it->first < id ||it->first == id )
            && ((std::next(it) == buckets.end()) || (id < std::next(it)->first)) ;
}

bool Bucket::onNewNode(const Sp<Node>& node, int confirm, bool isServer)
{

    auto b = findBucket(node->getId());
    if (b == buckets.end())
        return false;

    for (auto& n : b->nodes)
    {

        if( (n->getId() == node->getId())&&(!n->isExpired()))
            return false;
        /* Try to get rid of an expired node. */
        if (n->isExpired())
            {
                n = node;
                return true;
            }
    }

    bool mybucket = isServer?false:contains(b, selfId);
    if(mybucket)
        grow_time = clock::now();
    if (b->nodes.size() >= MAX_NODE)
    {
        /* Try to get rid of an expired node. */
        int dNum = isServer?11:6;
        if (mybucket || depth(b) < dNum)
        {
            split(b);
            return onNewNode(node, confirm,isServer);
        }
        QLOG_WARN()<<"drop new node for bucket full : ";
        node->getId().printNodeId(true);

        /* No space for this node.  Cache it away for later. */
        if (confirm or not b->cached)
            b->cached = node;
        int i = 0;
        for(auto& n : b->nodes)
        {
            i++;
            if(i == 8)
            {
                n = node;
                return true;
            }
        }
        QLOG_ERROR()<<"on new node error";
        return false;
    }
    else
    {
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
        if((id == n->getId())&&(!n->isExpired()))
            return true;
    }
    return false;
}

Sp<Node> Bucket::getNode(const NodeId &id)
{
    auto it = findBucket(id);
    for (auto & n : it->nodes)
    {
        if((id == n->getId())&&(!n->isExpired()))
            return n;
    }
    return nullptr;
}

bool Bucket::bucketMaintenance(sendNode sendFindNode, bool neighbour)
{
    Kbucket::iterator q;
    if(!neighbour)
        q = buckets.begin();
    else
        q=findBucket(selfId);

    Sp<Node> dstNode{};
    while ( q != buckets.end())
    {
        dstNode.reset();//bug
        int goodNode = 0;
        int badNode = 0 ;
        if(q->nodes.empty())
        {
            q = std::next(q);
            continue;
        }

        if(!neighbour && (q->lastTime < (clock::now() - seconds(5*buckets.size()))))
        {
            q=std::next(q);
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
        badNode = q->nodes.size() - goodNode;//bug     
        if(!neighbour && (badNode >goodNode )&&goodNode > 0)//bug
        {
            q->lastTime = clock::now();
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
                {
                    q->lastTime = clock::now();
                    return true;
                }
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


//xxn测试
std::list<Sp<Node>> Bucket::repNodes(const NodeId &id)
{
    std::list<Sp<Node>> returnNodes{};
    if (bucketIsEmpty(id))
    {
        return returnNodes;
    }
    auto it = findBucket(id);
    int bit = depth(it) - 1;
    NodeId id_first = it->first;
    auto insertNodes = [&](int k,NodeId id)
    {
        auto it = findBucket(id);
        if(it->first != id)
        {
            return 0;
        }
        int insertNumber = 0;
        for(auto &n : it->nodes)
        {
            if(!n->isExpired())
            {
                returnNodes.push_back(n);
                insertNumber ++;
                if(--k <= 0)
                    break;
            }
        }
        return insertNumber;
    };
    int k = 8;
    bool kIs1 = false;
    insertNodes(k,it->first);
    for (;  bit >= 0 ; bit--)
    {
        int num = 0;
        auto insertOther = [&,insertNodes](int i)
        {
            id_first[(bit + i) / 8] ^= (0x80 >> ((bit + i) % 8));
            auto thisNum = insertNodes(k, id_first);
            num += thisNum;
            return thisNum;
        };
        auto insertOtherOne = [&,insertOther](int i)
        {
            auto tmp = insertOther(i);
            if(kIs1 && !tmp)
            {
                insertOther(4);
                id_first[(bit + 4) / 8] ^= (0x80 >> ((bit + 4) % 8));
            }
        };
        insertOther(0);

        if(k < 8)
        {
            insertOther(1);

            if(k < 4)
            {
                insertOther(2);
                insertOther(1);
                if(k < 2)
                {
                    insertOtherOne(3);
                    insertOtherOne(2);
                    insertOtherOne(1);
                    insertOtherOne(2);
                    kIs1 = true;
                }
            }
        }
        id_first[bit / 8] &= (0xfe << (7 - bit % 8));
        id_first[bit / 8 + 1] = 0x00;
        if(k > 1)
            k /= 2;
    }
    return returnNodes;
}

std::list<Sp<Node>> Bucket::broadcastOthers(const NodeId &sourceId)
{
    int insertNum = 0;
    int dep = -1;
    std::list<Sp<Node>> returnNodes{};
    if(bucketIsEmpty(selfId))
    {
        QLOG_ERROR()<<"bucket is empty, broadcast to nobody!";
        return returnNodes;
    }
    auto it = findBucket(selfId);
    for(unsigned i = 0; i < ID_LENGTH; ++i)
    {
        if(sourceId[i] == selfId[i])
            continue;
        auto _xor = sourceId[i] ^ selfId[i];
        for(int j = 0; j < 8; ++j)
        {
            if(_xor & (0x80 >> j))
            {
                dep = j + 8 * i;
                break;
            }
        }
    }
    for(auto n = buckets.begin();; ++n)
    {
        if(n == buckets.end())
            n = buckets.begin();
        if(n->nodes.empty())
        {
            QLOG_ERROR()<<"bucket   "<<n->first.toString().c_str()<<"  is empty!";
            continue;
        }
        if(it->first != n->first && (int(depth(n)-1) == dep))
        {
            for(auto &n1 : n->nodes)
            {
                if(!n1->isExpired())
                {
                    returnNodes.push_back(n1);
                    insertNum++;
                    dep++;
                    break;
                }
            }
        }
        if(dep == int(depth(it)-1))
            break;
    }
    return returnNodes;
}


std::list<Sp<Node>> Bucket::broadcastLocal()
{
    int insertNum = 0;
    std::list<Sp<Node>> returnNodes{};
    if(bucketIsEmpty(selfId))
    {
        QLOG_ERROR()<<"bucket is empty";
        return returnNodes;
    }
    auto it = findBucket(selfId);

    if(it->nodes.empty())
        QLOG_ERROR()<<"bucket   "<<it->first.toString().c_str()<<"  is empty!";
    else
        ;
    for(auto &n : it->nodes)
    {
        if(!n->isExpired())
        {
            returnNodes.push_back(n);
            insertNum++;
        }
    }
    return returnNodes;
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
    new_first_id.printNodeId();
    // Insert new bucket
    buckets.insert(std::next(b), bucket {b->af, new_first_id,});//从next(b)之前开始插入，

    // Re-assign nodes重新分派节点
    std::list<Sp<Node>> nodes {};//std::shared_ptr<T>;
    nodes.splice(nodes.begin(), b->nodes);
    while (!nodes.empty())
    {
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

void Bucket::delExpNode()
{
    for(auto b = buckets.begin(); b != buckets.end(); b++)
    {
        for(auto it = b->nodes.begin(); it != b->nodes.end();)
        {
            auto itt = it++;
            if((*itt)->isExpired())
                b->nodes.erase(itt);
        }
    }
    QLOG_INFO()<<"expired check finished!";
}

void Bucket::delEmpBuk()
{
    bool unfinish = true;
    while(unfinish)
    {
        unfinish = false;
        for(auto b = buckets.begin(); b != buckets.end();)
        {
            if(b->nodes.size() == 0)
            {
                if(b == buckets.begin())
                {
                    b++;
                    continue;
                }
                int bit1 = std::prev(b)->first.lowBit();
                int bit2 = b->first.lowBit();
                int bit3 = std::next(b) != buckets.end() ? std::next(b)->first.lowBit() : -1;
                if(bit1 < bit2 && bit2 > bit3)
                {
                    auto itt = b++;
                    buckets.erase(itt);
                    unfinish = true;
                }
                else if(bit2 < bit3)
                {
                    if(bit3 != -1)
                    {
                        auto bit4 = std::next(std::next(b)) != buckets.end() ? std::next(std::next(b))->first.lowBit() : -1;
                        if(bit3 < bit4)
                        {
                            b++;
                            continue;
                        }
                        else if(bit3 == bit4)
                        {
                            QLOG_ERROR()<<"delete empty bucket first lowbit error2";
                            return;
                        }
                    }
                    std::next(b)->nodes.splice(b->nodes.begin(), b->nodes);
                    b++;
                    auto itt = b++;
                    buckets.erase(itt);
                    unfinish = true;
                }
                else if (bit2 == bit3)
                {
                    QLOG_ERROR()<<"delete empty bucket first lowbit error";
                    return;
                }
                else
                    b++;
            }
            else
                b++;
        }
    }
    QLOG_WARN()<<"out of for! delempbuk!";

}

bool Bucket::isEmpty() const
{
    return buckets.empty();
}

bool Bucket::bucketIsEmpty(NodeId id)
{
    return (buckets.size()==1) && (findBucket(id)->nodes.size()==0) ;
}

NodeId Bucket::getSelfId()
{
    return selfId;
}

void Bucket::dump(int type) const
{
    if(type == 0)//ALL
    {
        int i = 1,j = 0;
        QLOG_ERROR()<<"New table";
        for(auto& b: buckets)
        {
            QLOG_WARN()<<"NO."<<i++<<" bucket size= "<<b.nodes.size();
            b.first.printNodeId();
            for(auto& n: b.nodes)
            {
                n->getId().printNodeId(n->isExpired());
                if(!n->isExpired())
                    j+=1;
            }
        }
        QLOG_WARN()<<"k-bucket num = "<<buckets.size();
        QLOG_WARN()<<"good nodes number = "<<j;
        QLOG_ERROR()<<"self id is:";
        selfId.printNodeId();
    }
    else if(type == 1)//节点
    {
        int sum = 0;
        int good_sum = 0;
        for(auto& b: buckets)
        {
            for(auto& n: b.nodes)
            {
                if(!n->isExpired())
                    good_sum++ ;
            }
            sum += b.nodes.size();
        }

        QLOG_INFO()<<"Bucket summary info:";
        QLOG_INFO()<<"bucket num = "<<buckets.size();
        QLOG_INFO()<<"total node num = "<<sum;
        QLOG_INFO()<<"Good nodes num = "<<good_sum;
    }
    if(type == 3)//ALL
    {
        int i = 1,j = 0;
        QLOG_ERROR()<<"networkTable";
        for(auto& b: buckets)
        {
            for(auto& n: b.nodes)
            {
                n->getId().printNodeId(n->isExpired());
                if(!n->isExpired())
                    j+=1;
            }
        }
        QLOG_WARN()<<"good nodes number = "<<j;
        QLOG_ERROR()<<"self id is:";
        selfId.printNodeId();
    }
}

}
