/**
 * \file TSQueueTest.cpp
 **/

#include "AquetiToolsTest.h"

using namespace atl;

/**
* @brief Test: prints the size of the queue for debugging
*
* @param q The queue
* @param print False suppresses output
*/
void print_queue_size(TSQueue<int>& q, bool print)
{
    if (print) {
        std::cout << "Queue length" << q.size() << std::endl;
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
    if (print) {
        std::cout << "Adding " << num << " to queue" << std::endl;
    }
    q.enqueue(num);
}

/**
* @brief Removes the head of the queue and prints it for debugging
*
* @param q The queue
* @param print False suppresses output
*
* @return True when done testing, false if remove from queue fails
*/
bool remove_from_queue(TSQueue<int>& q, bool print)
{
    if (print) {
        std::cout << "Removing from queue" << std::endl;
    }
    int num;
    if (!q.dequeue(num, 1000)) {
        std::cout << "Failed to remove from queue" << std::endl;
        return false;
    }
    if(print) {
        std::cout << "Removed " << num << " from queue" << std::endl;
    }

    return true;
}

/**
* @brief Peeks at the head element in the queue
*
* @param q The queue
* @param print False suppresses output
*
* @return True if peek succeeds 
*/
bool peek_at_queue(TSQueue<int>& q, bool print)
{
    int num = 0;
    if (!q.peek(num)) {
        if (print) {
            std::cout << "Peek failed!" << std::endl;
        }
        return false;
    }
    if (print) {
        std::cout << "Head: " << num << std::endl;
    }

    return true;
}

/**
* @brief Pushes an element to the head of the queue
*
* @param q The queue
* @param num The number added to the queue
* @param print False suppresses output
*
* @return True when done testing
*/
bool push_to_queue(TSQueue<int>& q, int num, bool print)
{
    if (print) {
        std::cout << "Adding " << num << " to queue" << std::endl;
    }
    q.push(num);
    return true;
}

/**
* @brief Pops an element off the head of the queue
*
* @param q The queue
* @param print False suppresses output
*
* @return True when done testing, false if pop fails
*/
bool pop_from_queue(TSQueue<int>& q, bool print)
{
    if (print) {
        std::cout << "Popping from queue" << std::endl;
    }
    int num;
    if (!q.pop(num, 0)) {
        std::cout << "Failed to pop" << std::endl;
        return false;
    }
    if (print) {
        std::cout << "Popped " << num << " from queue" << std::endl;
    }
    return true;
}

namespace atl {

/**
* @brief Tests the thread-safe queue with multiple threads
*
* @param numThreads The number of threads to spawn
* @param printFlag False supresses output
*
* @return true on success
*/
JsonBox::Value testTSQueue(unsigned int numThreads, bool printFlag, bool assertFlag)
{
    TSQueue<int> q;
    JsonBox::Value resultString; //!< Brief JsonBox value with unit test results

    //Tests enqueue function
    for (int i = 0; i < 100; i++) {
        int value = i;
        if (!q.enqueue(value)) {
            if (printFlag) {
                std::cout << "Failed to enqueue to index " << i << std::endl;
                std::cout << "TSQueue test error. See TSQueueTest.log" << std::endl;
            }
            if (assertFlag) {
                assert(false);
            }
            resultString["Enqueue 1"] = "fail";
            resultString["pass"] = false;
        };
    }

    if(resultString["Enqueue 1"] != "fail"){
        resultString["Enqueue 1"] = "pass";
    }

    q.delete_all();

    //Tests size of 0
    unsigned size = q.size();
    if (size != 0) {
        if (printFlag) {
            std::cout << "Incorrect size(): " << size << " != 0" << std::endl;
            std::cout << "TSQueue test error. See TSQueueTest.log" << std::endl;
        }
        if (assertFlag) {
            assert(false);
        }
        resultString["Size 1"] = "fail";
        resultString["pass"] = false;
    } else {
        resultString["Size 1"] = "pass";
    }

    //Tests enqueue function again
    for (int i = 0; i < 100; i++) {
        int value = i;
        if (!q.enqueue(value)) {
            if (printFlag) {
                std::cout << "Failed to enqueue to index " << i << std::endl;
                std::cout << "TSQueue test error. See TSQueueTest.log" << std::endl;
            }
            if(assertFlag) {
                assert(false);
            }
            resultString["Enqueue 2"] = "fail";
            resultString["pass"] = false;
        };
    }

    if(resultString["Enqueue 2"] != "fail"){
        resultString["Enqueue 2"] = "pass";
    }

    //Tests size of 100
    size = q.size();
    if (size != 100) {
        if(printFlag) {
            std::cout << "Incorrect size(): " << size << " != 100" << std::endl;
            std::cout << "TSQueue test error. See TSQueueTest.log" << std::endl;
        }
        if (assertFlag) {
            assert(false);
        }
        resultString["Size 2"] = "fail";
        resultString["pass"] = false;
        return resultString;
    } else {
        resultString["Size 2"] = "pass";
    }

    //Tests dequeue function
    int result;
    for (int i = 0; i < 100; i++) {
        if (!q.dequeue(result)) {
            if (printFlag) {
                std::cout << "Failed to dqueue at index " << i << std::endl;
                std::cout << "TSQueue test error. See TSQueueTest.log" << std::endl;
            }
            if (assertFlag) {
                assert(false);
            }
            resultString["Dequeue1 "] = "fail";
            resultString["pass"] = false;
        };

        if (result != i) {
            if (printFlag) {
                std::cout << "Unexpected result: "<<result<<" != " << i << std::endl;
                std::cout << "TSQueue test error. See TSQueueTest.log" << std::endl;
            }
            if(assertFlag) {
                assert(false);
            }
            resultString["Dequeue 1"] = "fail";
            resultString["pass"] = false;
        };
    }

    if(resultString["Dequeue 1"] != "fail"){
        resultString["Dequeue 1"] = "pass";
    }

    if (q.dequeue(result, 1000)) { //Try to dequeue for a second
        if (printFlag) {
            std::cout << "Dequeue succeeded with no elements left" << std::endl;
            std::cout << "TSQueue test error. See TSQueueTest.log" << std::endl;
        }
        if (assertFlag) {
            assert(false);
        }
        resultString["Dequeue 2"] = "fail";
        resultString["pass"] = false;
    } else {
        resultString["Dequeue 2"] = "pass";
    }

    // Test thread safety
    std::thread* t = new std::thread[numThreads];
    if (printFlag) {
        std::cout << "Default max size: " << q.get_max_size() << std::endl;
    }
    q.set_max_size(10);

    if (printFlag) {
        std::cout << "New max size: " << q.get_max_size() << std::endl;
    }

    std::clock_t start;
    double duration = 0;

    if (printFlag) {
        std::cout << "Operations: " << numThreads << std::endl << "Threads: " << numThreads << std::endl;
    }

    start = std::clock();

    for (unsigned i = 0; i < numThreads; i++) {
        int num = (uint64_t)(getTime()* 1E6) % 6 + 1;
        std::cout << "Number:" << num << " on count " << i << "of " << numThreads << std::endl;
        switch (num) {
        case 1:
            t[i] = std::thread(add_to_queue, std::ref(q), i, printFlag);
            break;
        case 2:
            t[i] = std::thread(remove_from_queue, std::ref(q), printFlag);
            break;
        case 3:
            t[i] = std::thread(peek_at_queue, std::ref(q), printFlag);
            break;
        case 4:
            t[i] = std::thread(push_to_queue, std::ref(q), i, printFlag);
            break;
        case 5:
            t[i] = std::thread(pop_from_queue, std::ref(q), printFlag);
            break;
        case 6:
            t[i] = std::thread(print_queue_size, std::ref(q), printFlag);
            break;
        default:
            break;
        }
    }

    for (unsigned i = 0; i < numThreads; i++) {
        t[i].join();
    }

    if (printFlag) {
        std::cout << "Joined all" << std::endl;
    }

    duration = (std::clock() - start ) / (double) CLOCKS_PER_SEC;
    if(printFlag) {
        std::cout << "Time: " << duration << std::endl;
    }

    delete[] t;

    if (printFlag) {
        std::cout << "TSQueue Test Complete" << std::endl;
    }

    // result = std::system("rm TSQueueTest.log");
    // if (result != 0) {
    //     std::cout << "Failed to remove log" << std::endl;
    //     resultString["Remove log"] = "fail";
    //     resultString["pass"] = false;
    //     return resultString;
    // }

    if (resultString["pass"] == false) {
        return resultString;
    }
    resultString["pass"] = true;
    return resultString;
}
}
