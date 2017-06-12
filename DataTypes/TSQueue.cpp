//   Copyright Aqueti 2016.

// Distributed under the Boost Software License, Version 1.0.

//    (See accompanying file LICENSE_1_0.txt or copy at

//   http://www.boost.org/LICENSE_1_0.txt)

#include "TSQueue.tcc"
#include <cstdio>
#include <ctime>
#include <random>
#include <thread>
#include <assert.h>
#include <Timer.h>
#include <fstream>

using namespace aqt;
std::ofstream qTest;

/**
* @brief Test: prints the size of the queue for debugging
*
* @param q The queue
* @param print False suppresses output
*/
void print_size(TSQueue<int>& q, bool print)
{
    if(print) {
        qTest << "Queue length: " << q.size() << std::endl;
    } else {
        q.size();
    }
}

/**
* @brief Test: adds a number to the queue and prints for debugging
*
* @param q the queue
* @param num the number
* @param print False suppresses output
*/
void add_to_queue(TSQueue<int>& q, int num, bool print)
{
    if(print) {
        qTest << "Adding " << num << " to queue" << std::endl;
    }
    q.enqueue(num);
}

/**
* @brief Removes the head of the queue and prints it for debugging
*
* @param q The queue
* @param print False suppresses output
*/
bool remove_from_queue(TSQueue<int>& q, bool print)
{
    if(print) {
        qTest<< "Removing from queue" << std::endl;
    }
    int num;
    if( !q.dequeue(num, 1000)) {
        qTest << "Failed to remove from queue"<<std::endl;
        return false;
    }
    if(print) {
        qTest << "Removed " << num << " from queue" << std::endl;
    }

    return true;
}

/**
* @brief Peeks at the head element in the queue
*
* @param q The queue
* @param print False suppresses output
*/
bool peek_at_queue(TSQueue<int>& q, bool print)
{
    int num = 0;
    if( !q.peek( num )) {
        if(print) {
            qTest << "Peek failed!"<<std::endl;
        }
        return false;
    }
    if(print) {
        qTest << "Head: " << num << std::endl;
    }

    return true;
}

/**
* @brief Pushes an element to the head of the queue
*
* @param q The queue
* @param print False suppresses output
*/
bool push_to_queue(TSQueue<int>& q, int num, bool print)
{
    if(print) {
        qTest << "Adding " << num << " to queue" << std::endl;
    }
    q.push(num);
    return true;
}

/**
* @brief Pops an element off the head of the queue
*
* @param q The queue
* @param print False suppresses output
*/
bool pop_from_queue(TSQueue<int>& q, bool print)
{
    if(print) {
        qTest << "Popping from queue" << std::endl;
    }
    int num;
    if( ! q.pop(num, 0)) {
        qTest <<"Failed to pop"<<std::endl;
        return false;
    }
    if(print) {
        qTest << "Popped " << num << " from queue" << std::endl;
    }

    return true;
}

namespace aqt
{

/**
* @brief Tests the thread-safe queue with multiple threads
*
* @param numThreads The number of threads to spawn
* @param print False supresses output
*
* @return true on success
*/
bool testTSQueue(unsigned int numThreads, bool print, bool assertFlag)
{

    //create file to redirect output to
    qTest.open("TSQueueTest.log");

    TSQueue<int> q;

    for( int i = 0; i < 100; i++ ) {
        int value = i;
        if(!q.enqueue(value)) {
            if(print) {
                qTest << "Failed to enqueue to index "<<i<<std::endl;
                std::cout<<"TSQueue test error. See TSQueueTest.log" << std::endl;
            }
            if(assertFlag) {
                assert(false);
            }
            return false;
        }
    }

    q.delete_all();

    unsigned size = q.size();
    if(size != 0) {
        if(print) {
            qTest << "Incorrect size(): " << size << " != 0"<<std::endl;
            std::cout<<"TSQueue test error. See TSQueueTest.log" << std::endl;
        }
        if(assertFlag) {
            assert(false);
        }
    };

    for( int i = 0; i < 100; i++ ) {
        int value = i;
        if(!q.enqueue(value)) {
            if(print) {
                qTest << "Failed to enqueue to index "<<i<<std::endl;
                std::cout<<"TSQueue test error. See TSQueueTest.log" << std::endl;
            }
            if(assertFlag) {
                assert(false);
            }
            return false;
        }
    }

    size = q.size();
    if(size != 100) {
        if(print) {
            qTest << "Incorrect size(): " << size << " != 100"<<std::endl;
            std::cout<<"TSQueue test error. See TSQueueTest.log" << std::endl;
        }
        if(assertFlag) {
            assert(false);
        }
    }

    int result;
    for(int i = 0; i < 100; i++ ) {
        if( !q.dequeue(result)) {
            if(print) {
                qTest << "Failed to dqueue at index "<<i<<std::endl;
                std::cout<<"TSQueue test error. See TSQueueTest.log" << std::endl;
            }
            if(assertFlag) {
                assert(false);
            }
            return false;
        }

        if( result != i ) {
            if(print) {
                qTest << "Unexpected result: "<<result<<" != "<<i<<std::endl;
                std::cout<<"TSQueue test error. See TSQueueTest.log" << std::endl;
            }
            if(assertFlag) {
                assert(false);
            }
            return false;
        }
    }

    if(q.dequeue(result, 1000)) { //Try to dequeue for a second
        if(print) {
            qTest << "Dequeue succeeded with no elements left"<<std::endl;
            std::cout<<"TSQueue test error. See TSQueueTest.log" << std::endl;
        }
        if(assertFlag) {
            assert(false);
        }
    }

    // Test thread safety
    std::thread* t = new std::thread[numThreads];
    if(print) {
        qTest << "Default max size: " << q.get_max_size() << std::endl;
    }
    q.set_max_size(10);

    if(print) {
        qTest << "New max size: " << q.get_max_size() << std::endl;
    }

    std::clock_t start;
    double duration = 0;

    if(print) {
        qTest << "Operations: " << numThreads << std::endl << "Threads: " << numThreads << std::endl;
    }

    start = std::clock();

    for(unsigned i = 0; i < numThreads; i++) {
        int num = (uint64_t)(getTime()* 1E6)%6+1;
        qTest <<"Number:"<<num<<" on count "<<i<<"of "<<numThreads<<std::endl;
        switch( num ) {
        case 1:
            t[i] = std::thread(add_to_queue, std::ref(q), i, print);
            break;
        case 2:
            t[i] = std::thread(remove_from_queue, std::ref(q), print);
            break;
        case 3:
            t[i] = std::thread(peek_at_queue, std::ref(q), print);
            break;
        case 4:
            t[i] = std::thread(push_to_queue, std::ref(q), i, print);
            break;
        case 5:
            t[i] = std::thread(pop_from_queue, std::ref(q), print);
            break;
        //case 6:
          //  t[i] = std::thread(print_size, std::ref(q), print);
            //break;
        default:
            break;
        }
    }

    for(unsigned i=0; i < numThreads; i++) {
        t[i].join();
    }
    if(print) {
        qTest << "Joined all"<<std::endl;
    }

    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
    if(print) {
        qTest << "Time: " << duration << std::endl;
    }


    delete[] t;


    if(print) {
        std::cout <<"TSQueue Test Complete"<<std::endl;
    }

    if(print) {
        std::cout <<"TSQueue Test Complete"<<std::endl;
    }

    std::system("rm TSQueueTest.log");
    return true;
}
}
