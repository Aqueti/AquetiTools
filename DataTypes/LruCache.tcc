#pragma once

#include "TSQueue.tcc"
#include <map>

namespace aqt
{

/**
 * @brief This node provides access to the key so that when something is ejected from the cache
 *      we will be able to intelligently remove its key from the Queue.
 *
 */
template<class K, class V> struct CacheNode {
    //TODO: Try to add this to the LruCache class.  Probably won't work due to inheritance
    K key;
    V value;
};

/**
 * @brief A thread-safe LRU Cache implementaiton.
 *
 * @tparam K The key class used to access elements
 * @tparam V The cached object type
 */
template<class K, class V> class LruCache: protected TSQueue<CacheNode<K,V>>
{
private:
    typedef TSQueue<CacheNode<K,V>> Q;

public:
    virtual ~LruCache();
    virtual bool add_to_cache(K, V);
    virtual bool get_value(K, V&);
    virtual bool get_lower_bound(K, V&);
    virtual void empty_cache();
    virtual void setCleanupHandler(std::function<bool(K,V)> handler=nullptr);
    using Q::size;
    using Q::set_max_size;
    using Q::get_max_size;

protected:
    typedef typename Q::QNode QNode;      //<! Defining QNode
    virtual bool push_to_back(std::shared_ptr<QNode>);
    std::map<K, std::weak_ptr<QNode>> keyMap;         // map of Key to weak pointer to QNodes
    std::function<bool(K,V)> m_cleanupHandler;
};

bool test_LruCache(unsigned int numThreads, bool print, bool assertFlag);

/*
 * @brief Destructor.  Calls empty_cache()
 */
template<class K, class V> LruCache<K,V>::
~LruCache()
{
    empty_cache();
}

/**
 * @brief Empties the Queue and the map.
 */
template<class K, class V> void LruCache<K,V>::
empty_cache()
{
    std::lock_guard<std::recursive_mutex> lock(Q::m);
    keyMap.clear();
    Q::delete_all();   //Recursive mutex allows for multiple locks from the same thread
}

/**
 * @brief Sets the function that is called when something is booted from the cache
 *
 * If this function returns false when called, the object will not be booted from the cache!
 *
 * @param std::function handler the function
 */
template<class K, class V> void LruCache<K,V>::
setCleanupHandler(std::function<bool(K,V)> handler)
{
    m_cleanupHandler = handler;
}

/**
 * @brief Retrieves the value pointed to by this key
 *
 * @param key The key
 * @param[in] val The return value
 *
 * @return true on success.  False if no key exists or a cache miss
 */
template<class K, class V> bool LruCache<K,V>::
get_value(K key, V& val)
{
    std::lock_guard<std::recursive_mutex> lock(Q::m);
    if(!keyMap.count(key)) {
        return false;
    } else if (keyMap[key].expired()) {
        std::cerr << "WE SHOULDN'T SEE THIS OR WE HAVE A PROBLEM" << std::endl;
        std::cerr << "map size: " << keyMap.size() << " length: " << Q::length;
        auto node = Q::head;
        int i=0;
        while(node) {
            i++;
            node = node->prev;
        }
        keyMap.erase(key);
        //std::cout << " real len: " << i << std::endl;
        return false;
    }
    auto node = keyMap[key].lock();
    val = node->data.value;
    push_to_back(node);
    return true;
}

/**
 * @brief Retrieves the value pointed to by this key
 *
 * @param key The key
 * @param[in] val The return value
 *
 * @return true on success.  False if no key exists or a cache miss
 */
template<class K, class V> bool LruCache<K,V>::
get_lower_bound(K key, V& val)
{
    std::lock_guard<std::recursive_mutex> lock(Q::m);
    if( keyMap.lower_bound(key) == keyMap.end() ) {
        return false;
    } else if (keyMap.lower_bound(key)->second.expired()) {
        std::cerr << "WE SHOULDN'T SEE THIS OR WE HAVE A PROBLEM" << std::endl;
        std::cerr << "map size: " << keyMap.size() << " length: " << Q::length;
        auto node = Q::head;
        int i=0;
        while(node) {
            i++;
            node = node->prev;
        }
        keyMap.erase(key);
        //std::cout << " real len: " << i << std::endl;
        return false;
    }
    auto node = keyMap.lower_bound(key)->second.lock();
    val = node->data.value;
    push_to_back(node);
    return true;
}

/**
 * @brief This function creates a CacheNode which contains a key and a value,
 *      then adds it to the queue.
 *
 * @param key The key
 * @param value The value
 * @param force true will push data even if the length is greater than max_size
 *
 * @return
 */
template<class K, class V> bool LruCache<K,V>::
add_to_cache(K key, V value)
{
    std::unique_lock<std::recursive_mutex> lock(Q::m);

    //Check to see if something needs booted
    int count = 0;
    while(Q::length >= Q::max_size && count < 5) {  //Try to boot 5 times.  If still too big, give up.
        if(!Q::head) {
            std::cerr << "LruCache::add_to_cache ERROR: Head is null but length is non-zero." << std::endl;
            break;
        }
        std::shared_ptr<QNode> temp = Q::head;
        Q::head = Q::head->prev;
        temp->prev = nullptr;
        if(Q::head) {
            Q::head->next.reset();
        }
        Q::length--;
        if(keyMap.erase(temp->data.key) == 0) {
            std::cerr << "LruCache::add_to_cache ERROR: keyMap could not find key" << std::endl;
            //std::cout << "length: " << Q::length << std::endl;
            //std::cout << "size: " << keyMap.size() << std::endl;
            auto it = Q::head;
            int i = 0;
            while(it != nullptr) {
                it = it->prev;
                i++;
            }
            //std::cout << "actual length: " << i << std::endl;
        }

        if(m_cleanupHandler) {
            lock.unlock();
            bool boot = m_cleanupHandler(temp->data.key, temp->data.value);
            lock.lock();
            if(!boot) {
                auto it = keyMap.emplace(temp->data.key, temp);
                if(it.second) {
                    Q::enqueue(temp);
                } else {
                    std::cerr << "LruCache::add_to_cache ERROR: couldn't resubmit after failed boot" << std::endl;
                }
                count++;
                continue;
            }
        }
    }

    // Create a CacheNode to put in the queue
    CacheNode<K,V> node;
    node.key = key;
    node.value = value;

    // Create a QNode pointer out of that CacheNode
    std::shared_ptr<QNode> temp = std::shared_ptr<QNode>(new QNode(node));

    // Put the key in the keySet
    auto it = keyMap.emplace(key, temp);
    if(!it.second) {
        std::cerr << "LruCache::add_to_cache ERROR: Failed to add element to map." << std::endl;
        return false;
    }

    // Enqueue the CacheNode and notify of a new object in the queue
    Q::enqueue(temp); //Recursive mutex allows for multiple locks from the same thread
    Q::enqueue_cv.notify_one();
    return true;
}

/**
 * @brief This function moves a node to the back of the queue.
 *
 *      This function assumes the mutex has been locked before being called
 *
 * @param node The node to move
 *
 * @return false if the node was already at the back
 */
template<class K, class V> bool LruCache<K,V>::
push_to_back(std::shared_ptr<QNode> node)
{
    if(node->prev == nullptr) {
        return false;    //Already at the back of the queue
    }
    // So we KNOW node->prev exists

    if(auto next = node->next.lock()) { // If there exists a next node, snatch it
        next->prev = node->prev;        // and point it's prev pointer to node->prev
    } else {
        Q::head = node->prev;              // If not, this must be head. Set it to node->prev
    }

    node->prev->next = node->next;      //already checked for prev.  set it's next to node->next

    node->next = Q::tail;                  // Point node to previous tail
    if(auto t = Q::tail.lock()) {
        t->prev = node;                 // Point previous tail to node
    } else {
        std::cerr << "ERROR: Tail pointer could not be found" << std::endl;
    }

    Q::tail = node;                     // Set tail to node
    node->prev = nullptr;               // Set end of queue to nullptr

    return true;
}
}
