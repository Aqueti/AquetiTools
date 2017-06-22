#include <iostream>
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <memory>
#include "JsonBox.h"

#pragma once

#define DEFAULT_MAX_SIZE SIZE_MAX

namespace atl
{

/**
* @brief A thread-safe queue for APL commands between threads
*
* There must be a destructor for any class used in this template, or
* else there will be a memory leak if the Queue is deleted.
*
* @tparam T The type of data to be contained in the queue
*/
template <typename T> class TSQueue
{
protected:
    std::recursive_mutex m;                 //<! The mutex that will be used for accessing the queue
    std::condition_variable_any enqueue_cv; //<! The condition variable which waits on blocking dequeue
    std::condition_variable_any dequeue_cv; //<! The condition variable which waits on blocking dequeue
    std::atomic_size_t length;              //<! The length of the queue
    struct QNode;                           //<! A simple linked list node
    std::shared_ptr<QNode> head;            //<! The head of the queue
    std::weak_ptr<QNode> tail;              //<! The tail of the queue
    size_t max_size = DEFAULT_MAX_SIZE;     //<! Maximum size of queue

    virtual void enqueue(std::shared_ptr<QNode> node);   //<! Adds a QNode to the tail of the queue


public:
    TSQueue();                                          //<! Constructor
    virtual ~TSQueue();                                 //<! Destructor.  Deletes all data in queue
    virtual bool enqueue(const T&, bool force=false);   //<! Add data to the tail of the queue
    virtual bool dequeue(T& data, uint16_t timeout=0);  //<! Remove and return data from the head of the queue
    virtual bool push(T, bool force=false);             //<! Add data to the head of the queue (as a stack)
    virtual bool pop(T& data, uint16_t timeout=0);      //<! Pop data off the head of the queue (as a stack)
    virtual bool peek(T& value, uint16_t timeout=0);    //<! Peek at the head of the queue
    virtual size_t size();                              //<! Return the size of the queue
    virtual void delete_all();                          //<! Deletes all nodes in the queue
    virtual void set_max_size(size_t);                  //<! Sets max size
    virtual size_t get_max_size();                      //<! Returns max size
    virtual bool wait_until_empty(uint16_t timeout=0);  //<! Waits until queue is empty
};

//template<class K, class V> struct CacheNode;
//template class TSQueue<CacheNode<int,int>>;

JsonBox::Value testTSQueue(unsigned int numThreads = 20, bool print = false, bool assertFlag = false);

/**
* @brief Node struct for linked list
*/
template<typename T> struct TSQueue<T>::QNode {
    QNode(const T& new_data) {
        data = new_data;
    }
    T data;
    std::weak_ptr<QNode> next;   // Node closer to head
    std::shared_ptr<QNode> prev; // Node closer to tail
};

/**
* @brief Constructor
**/
template<typename T> TSQueue<T>::TSQueue(): length(0) {}

/**
* @brief Destructor.  Deletes all nodes in the queue
**/
template<typename T> TSQueue<T>::~TSQueue()
{
    delete_all();
}

/**
* @brief Deletes all nodes in the queue including their data
*/
template<typename T> void TSQueue<T>::delete_all()
{
    std::lock_guard<std::recursive_mutex> lock(m);
    head.reset();
    length = 0;
    dequeue_cv.notify_all();
}

/**
* @brief Adds a node to the tail of the queue, ignoring size and other checks.
*
* @param data The data to be contained in the Node
*/
template<typename T> void TSQueue<T>::enqueue(std::shared_ptr<QNode> node)
{
    std::lock_guard<std::recursive_mutex> lock(m);
    if(auto tailPtr = tail.lock()) {
        tailPtr->prev = node;
        node->next = tailPtr;
    } else {
        head = node;
    }
    tail = node;

    length++;
}

/**
* @brief Adds a node to the tail of the queue
*
* @param data The data to be contained in the Node
* @param force True will push data even if the length is greater than max_size
*/
template<typename T> bool TSQueue<T>::enqueue(const T& data, bool force)
{
    std::lock_guard<std::recursive_mutex> lock(m);
    if(!force && length >= max_size) {
        return false;
    }

    std::shared_ptr<QNode> temp = std::shared_ptr<QNode>(new QNode(data));
    enqueue(temp);      //Recursive mutex allows for multiple locks from the same thread
    enqueue_cv.notify_one();
    return true;
}

/**
* @brief Removes and returns the head of the queue.  Blocks if no data is available
* @param timeout How long to block before timeout in milliseconds.
*
* @return The data contained in the head
*/
template<typename T> bool TSQueue<T>::dequeue(T& data, uint16_t timeout)
{
    std::unique_lock<std::recursive_mutex> lock(m);
    if(!enqueue_cv.wait_for(lock, std::chrono::milliseconds(timeout), [this] {return length > 0;})) {
        return false;
    }
    data = head->data;
    head = head->prev;
    length--;
    if(!length) {
        dequeue_cv.notify_all();
    }
    return true;
}

/**
 * @brief Waits until the queue is empty, then returns.
 * NOTE: Due to the uncertain nature of multithreaded programming,
 * by the time this function returns, new objects may have been added
 *
 * @param timeout the maximum number of milliseconds to wait.
 * NOTE: A timeout of 0 will wait indefinitely.
 *
 * @return true if queue got to 0, false if timeout occured
 */
template<typename T> bool TSQueue<T>::wait_until_empty(uint16_t timeout)
{
    std::unique_lock<std::recursive_mutex> lock(m);

    if(!timeout) {
        dequeue_cv.wait(lock, [this] {return length == 0;});
        return true;
    }

    if(!dequeue_cv.wait_for(lock, std::chrono::milliseconds(timeout),
                [this] {return length == 0;})) {
        return false;
    }
    return true;
}

/**
* @brief Adds a node to the head of the queue (as a stack)
*
* @param data The data to be contained in the Node
* @param force True will push data even if the length is greater than max_size
*/
template<typename T> bool TSQueue<T>::push(T data, bool force)
{
    std::unique_lock<std::recursive_mutex> lock(m);
    if(!force && length >= max_size) {
        return false;
    }

    std::shared_ptr<QNode> temp = std::shared_ptr<QNode>(new QNode(data));
    if(head) {
        head->next = temp;
        temp->prev = head;
    } else {
        tail = temp;
    }
    head = temp;

    length++;
    enqueue_cv.notify_one();
    return true;
}

/**
* @brief Removes and returns the head of the queue (stack notation)
* @param timeout How long to block before timeout in milliseconds.
*
* @return The data contained in the head
*/
template<typename T> bool TSQueue<T>::pop( T& data, uint16_t timeout)
{
    return dequeue( data, timeout);
}

/**
* @brief Returns the data in the head of the queue without removing it.  Blocks if no data is available
* @param timeout How long to block before timeout in milliseconds.
*
* @return The data in the head of the queue
*/
template<typename T> bool TSQueue<T>::peek(T& value, uint16_t timeout)
{
    std::unique_lock<std::recursive_mutex> lock(m);
    if(!enqueue_cv.wait_for(lock, std::chrono::milliseconds(timeout), [this] {return length > 0;})) {
        return false;
    }

    value = head->data;
    return true;
}

/**
* @brief Returns the number of items in the queue
*
* @return the number of items in the queue
*/
template<typename T> size_t TSQueue<T>::size()
{
    return length;
}

/**
* @brief Sets the maximum size of the queue.  If the queue is currently
*           longer than the max size, the queue will not be modified,
*           but no more elements will be able to be added until the queue
*           is shorter than the max_size
*/
template<typename T> void TSQueue<T>::set_max_size(size_t size)
{
    std::lock_guard<std::recursive_mutex> lock(m);
    max_size = size;
}

/**
* @brief Returns the number of items in the queue
*
* @return the number of items in the queue
*/

template<typename T> size_t TSQueue<T>::get_max_size()
{
    std::lock_guard<std::recursive_mutex> lock(m);
    return max_size;
}
}
