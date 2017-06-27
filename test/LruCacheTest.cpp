#include "AquetiToolsTest.h"

std::mutex printMutex;

namespace atl {

/**
* @brief Test: prints the size of the cache for debugging
*
* @param cache The cache
* @param print False suppresses output
*/
void print_size(LruCache<int,std::string>& cache, bool print, std::shared_future<void> start)
{
    start.wait();

    if(print) {
        std::lock_guard<std::mutex> l(printMutex);
        std::cout << "Queue length: " << cache.size() << std::endl;
    } else {
        cache.size();
    }
}

/**
* @brief Test: adds a key and value to the cache and prints for debugging
*
* @param cache the cache
* @param num the number
* @param val the value
* @param print False suppresses output
*/
void add_to_cache(LruCache<int,std::string>& cache, int num, std::string val, bool print, std::shared_future<void> start)
{
    start.wait();

    if (print) {
        std::lock_guard<std::mutex> l(printMutex);
        std::cout << "Adding " << num << ": " << val << " to cache" << std::endl;
    }
    cache.add_to_cache(num, val);
}

/**
* @brief Retrieves and prints the value associated with the specified key
*
* @param cache The cache
* @param print False suppresses output
*/
bool retrieve_from_cache(LruCache<int,std::string>& cache, int key, bool print, std::shared_future<void> start)
{
    start.wait();

    if (print) {
        std::lock_guard<std::mutex> l(printMutex);
        std::cout << "Retrieving from cache" << std::endl;
    }
    std::string result;
    if (!cache.get_value(key, result)) {
        std::lock_guard<std::mutex> l(printMutex);
        std::cout << "Failed to retrieve " << key << " from cache" << std::endl;
        return false;
    }
    if (print) {
        std::lock_guard<std::mutex> l(printMutex);
        std::cout << "Retrieved " << key << ": " << result << " from cache" << std::endl;
    }

    return true;
}

/**
* @brief Pops an element off the head of the cache
*
* @param cache The cache
* @param print False suppresses output
*/
bool clear_cache(LruCache<int,std::string>& cache, bool print, std::shared_future<void> start)
{
    start.wait();

    if (print) {
        std::lock_guard<std::mutex> l(printMutex);
        std::cout << "Emptying cache" << std::endl;
    }
    cache.empty_cache();

    if (print) {
        std::lock_guard<std::mutex> l(printMutex);
        std::cout << "Cache empty" << std::endl;
    }
    return true;
}

/**
 * @brief Cleanup handler.  This randomly decides whether or not to boot a node from cache
 *
 * @return true to boot, false to not
 */
bool handler(int, std::string)
{
    return rand() % 2;
}

/**
 * @brief Tests the cache functions with and without threads
 *
 * @return true on success
 */
JsonBox::Value testLruCache(unsigned int numThreads, bool printFlag, bool assertFlag)
{
    JsonBox::Value resultString;
    LruCache <int, std::string> cache;
    cache.set_max_size(50);

    //Test add function
    for (int i = 0; i < 100; i++) {
        int value = i;
        if (!cache.add_to_cache(value, "string " + std::to_string(i))) {
            if (printFlag) {
                std::lock_guard<std::mutex> l(printMutex);
                std::cout << "Failed to add index "<<i<<" to cache"<<std::endl;
                std::cout<<"LruCache test error. See LruCacheTest.log" << std::endl;
            }
            if (assertFlag) {
                assert(false);
            }
            resultString["Add"] = "fail";
            resultString["pass"] = false;
            return resultString;
        }
    }
    resultString["Add"] = "pass";

    //Test size
    unsigned size = cache.size();
    if (size != 50) {
        if (printFlag) {
            std::lock_guard<std::mutex> l(printMutex);
            std::cout << "Incorrect size(): " << size << " != 50"<<std::endl;
            std::cout<<"LruCache test error. See LruCacheTest.log" << std::endl;
        }
        if (assertFlag) {
            assert(false);
        }
        resultString["Size"] = "fail";
        resultString["pass"] = false;
        return resultString;
    };
    resultString["Size"] = "pass";

    //Test retrieve function
    std::string result;
    for (int i = 75; i > 25; i--) {
        std::string result;
        bool rc = cache.get_value(i, result);

        std::cout << "Retrieve value: " << i << ": " << result << std::endl;
        int value = -1;

        if (rc) {
            value = std::stoi(result.substr(7));
            if (value != i) {
                std::lock_guard<std::mutex> l(printMutex);
                if (printFlag) {
                    std::cout << "Index " << i << "not the same as retrieved value"<<std::endl;
                    std::cout<<"LruCache test error. See LruCacheTest.log" << std::endl;                   
                }
                if(assertFlag) {
                    assert(false);
                }
                resultString["Retrieve"] = "fail";
                resultString["pass"] = false;
                return resultString;
            }
        }

        if ((!rc && value >= 50) || (rc && value < 50)) {
            if (printFlag) {
                std::lock_guard<std::mutex> l(printMutex);
                std::cout << "Failed to retrieve index "<<i<<" to cache"<<std::endl;
                std::cout<<"LruCache test error. See LruCacheTest.log" << std::endl;
            }
            if(assertFlag) {
                assert(false);
            }
            resultString["Retrieve"] = "fail";
            resultString["pass"] = false;
            return resultString;
        }
    }
    resultString["Retrieve"] = "pass";

    //Test max size add function
    cache.set_max_size(40);
    if (!cache.add_to_cache(1, "string 1")) {
        if (printFlag) {
            std::lock_guard<std::mutex> l(printMutex);
            std::cout << "Failed to add 1 to cache after changing max size"<<std::endl;
            std::cout<<"LruCache test error. See LruCacheTest.log" << std::endl;
        }
        if (assertFlag) {
            assert(false);
        }
        resultString["Max size add"] = "fail";
        resultString["pass"] = false;
        return resultString;
    }
    resultString["Max size add"] = "pass";

    //Test retrieve with changed max size
    for (int i = 99; i >= 50; i--) {
        std::string result;
        bool rc = cache.get_value(i, result);

        std::cout << "Retrieve value: " << i << ": " << result << std::endl;
        int value = -1;

        if (rc) {
            value = std::stoi(result.substr(7));
            if (value != i) {
                std::lock_guard<std::mutex> l(printMutex);
                if (printFlag) {
                    std::cout << "Index " << i << "not the same as retrieved value" << std::endl;
                    std::cout << "LruCache test error. See LruCacheTest.log" << std::endl;                   
                }
                if (assertFlag) {
                    assert(false);
                }
                resultString["Retrieve 2"] = "fail";
                resultString["pass"] = false;
                return resultString;
            }
        }

        if ((!rc && (i > 86 || i <= 75)) || (rc && i < 86 && i > 75)) {
            if (printFlag) {
                std::lock_guard<std::mutex> l(printMutex);
                std::cout << "Failed to retrieve index " << i <<" to cache after changing max size" << std::endl;
                std::cout<<"LruCache test error. See LruCacheTest.log" << std::endl;
            }
            if (assertFlag) {
                assert(false);
            }
            resultString["Retrieve 2"] = "fail";
            resultString["pass"] = false;
            return resultString;
        }
    }
    resultString["Retrieve 2"] = "pass";

    //Test empty cache size
    cache.empty_cache();

    size = cache.size();
    if (size != 0) {
        if (printFlag) {
            std::lock_guard<std::mutex> l(printMutex);
            std::cout << "Incorrect size(): " << size << " != 0"<<std::endl;
            std::cout<<"LruCache test error. See LruCacheTest.log" << std::endl;
        }
        if(assertFlag) {
            assert(false);
        }
        resultString["Empty cache size"] = "fail";
        resultString["pass"] = false;
        return resultString;
    }
    resultString["Empty cache size"] = "pass";

    // Test thread safety
    std::thread* t = new std::thread[numThreads];
    if (printFlag) {
        std::lock_guard<std::mutex> l(printMutex);
        std::cout << "Default max size: " << cache.get_max_size() << std::endl;
    }
    cache.set_max_size(10);

    if (printFlag) {
        std::lock_guard<std::mutex> l(printMutex);
        std::cout << "New max size: " << cache.get_max_size() << std::endl;
    }

    std::clock_t start;
    double duration = 0;

    if (printFlag) {
        std::lock_guard<std::mutex> l(printMutex);
        std::cout << "Operations: " << numThreads << std::endl << "Threads: " << numThreads << std::endl;
    }

    srand(time(0));

    std::promise<void> promise;
    std::shared_future<void> future(promise.get_future());  // All this does is make the threads wait to run at the same time
    cache.setCleanupHandler(handler);

    start = std::clock();

    for (unsigned i = 0; i < numThreads; i++) {
        unsigned num = rand() % 4;
        switch (num) {
        case 0:
            t[i] = std::thread(add_to_cache, std::ref(cache), i, "string " + std::to_string(i), printFlag, future);
            break;
        case 1:
            t[i] = std::thread(retrieve_from_cache, std::ref(cache), rand() % (i + 1), printFlag, future);
            break;
        case 2:
            t[i] = std::thread(clear_cache, std::ref(cache), printFlag, future);
            break;
        case 3:
            t[i] = std::thread(print_size, std::ref(cache), printFlag, future);
            break;
        default:
            std::cerr << "ERROR: Random number generator is producing too-large numbers" << std::endl;
            break;
        }
    }

    promise.set_value();  // Open the floodgate

    if (printFlag) {
        std::lock_guard<std::mutex> l(printMutex);
        std::cout << "Joining"<<std::endl;
    }

    for (unsigned i = 0; i < numThreads; i++) {
        if (t[i].joinable()) {
            t[i].join();
        } else {
            std::lock_guard<std::mutex> l(printMutex);
            std::cerr << "WARNING: thread " << i << " not joinable" << std::endl;
        }
    }

    if (printFlag) {
        std::lock_guard<std::mutex> l(printMutex);
        std::cout << "Joined all" << std::endl;
    }

    duration = (std::clock() - start) / (double) CLOCKS_PER_SEC;
    if (printFlag) {
        std::lock_guard<std::mutex> l(printMutex);
        std::cout << "Time: " << duration << std::endl;
    }

    delete [] t;

    if (printFlag) {
        std::lock_guard<std::mutex> l(printMutex);
        std::cout << "LruCache Test Complete" << std::endl;
    }

    resultString["pass"] = true;
    return resultString;
}
}