/**
 * \file MultiThread.cpp
 **/

#include "MultiThread.h"
#include <iostream>
#include <fstream>
#include <future>
#include <system_error>

std::ofstream multiThreadTest;

namespace atl
{

/**
 * \brief  Destructor
 *
 * This destructor detaches all threads
 **/
MultiThread::~MultiThread()
{
    MultiThread::Stop();
    MultiThread::Join();
}

/**
 * @brief Starts as many threads as were set in the constructor
 * or most recent setNumThreads call.
 *
 * @param runFlag
 *
 * @return
 */
bool MultiThread::Start()
{
    bool running = false;
    m_running.compare_exchange_strong(running, true);

    if(running) {
        std::cerr << "WARNING: Threads already running." << std::endl;
        return false;
    }

    std::promise<void> p;
    auto f = std::make_shared<std::shared_future<void>>(p.get_future().share());
    for(unsigned i = 0; i < m_numThreads; i++) {
        m_threads.emplace_back([this, f] {f->get(); Execute();});
        m_idMap.emplace(m_threads.crbegin()->get_id(), i);
    }
    p.set_value();
    return true;
}

/**
 * @brief Waits for all threads to complete before returning
 *
 * @return true if all threads were able to join
 */
bool MultiThread::Join()
{
    // Move threads into separate vector atomically before joining
    // Don't want to join from within mutex lock
    std::unique_lock<std::mutex> guard (m_threadMutex);
    auto deleteThreads = std::move(m_threads);
    guard.unlock();

    bool rc = true;
    for(auto&& t: deleteThreads) {
        try {
            auto id = t.get_id();
            t.join();
            // Don't erase detached threads
            m_idMap.erase(id);

        } catch (const std::system_error& e) {
            if(e.code() == std::errc::resource_deadlock_would_occur) {
                std::cerr << "Warning: Can't join yourself! detaching..." << std::endl;
                /*try {
                    t.detach();
                } catch (const std::system_error& e) {
                    std::cerr << "Warning: Join could not detach: " << e.what() << std::endl;
                    rc = false;
                }*/
            } else if(e.code() == std::errc::no_such_process) {
                std::cerr << "Warning: Thread not valid: " << e.what() << std::endl;
                rc = false;
            } else if(e.code() == std::errc::invalid_argument) {
                std::cerr << "Warning: Thread not joinable: " << e.what() << std::endl;
                std::cerr << "t.id: " << t.get_id() << std::endl;
                std::cerr << "me.id: " << std::this_thread::get_id() << std::endl;
                std::cerr << "joinable: " << t.joinable() << std::endl;
                rc = false;
            } else {
                std::cerr << "Warning: Unknown thread error occurred: " << e.what() << std::endl;
                rc = false;
            }
        }
    }

    // no need to set running false.  If joined, must be false

    return rc;
}

/**
 * @brief Detaches all threads and removes them from map
 *
 * @return true if all threads were able to detach
 */
bool MultiThread::Detach()
{
    std::unique_lock<std::mutex> guard (m_threadMutex);
    auto deleteThreads = std::move(m_threads);
    guard.unlock();

    bool rc = true;
    for(auto&& t: deleteThreads) {
        try {
            t.detach();
        } catch (const std::system_error& e) {
            std::cerr << "Warning: Detach could not detach: " << e.what() << std::endl;
            rc = false;
        }
    }

    // Keeping idmap around in case it's used

    return rc;
}

/**
 * @brief This sets the number of threads for the next time Start is called.
 * No currently-running threads will be removed by this call.  In order
 * to reduce the number of threads, you must call Stop(), then Start() again.
 *
 * @param numThreads the number of threads.
 *
 * @return true on success
 */
bool MultiThread::setNumThreads(unsigned numThreads)
{
    m_numThreads = numThreads;
    return true;
}

/**
 * @brief Returns a thread ID for this thread.  Will be an int between 0 and n-1,
 * where n is the number of threads
 *
 * @return -1 on failure, or an int between 0 and n-1
 */
int MultiThread::getMyId()
{
    if(!m_idMap.count(std::this_thread::get_id())) {
        std::cerr << "ERROR: called getMyId from thread outside of MultiThread" << std::endl;
        return -1;
    }
    return m_idMap[std::this_thread::get_id()];
}
}
