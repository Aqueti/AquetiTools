/**
 * \file MultiThread.cpp
 **/

#include "MultiThread.h"
#include <iostream>
#include <fstream>
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
    Stop();
    Join();
}

/**
 * @brief Starts as many threads as were set in the constructor
 * or most recent setNumThreads call.
 *
 * @param runFlag
 *
 * @return
 */
bool MultiThread::Start(bool* runFlag)
{
    if(m_running && *m_running) {
        std::cerr << "WARNING: Threads already running." << std::endl;
        return false;
    }

    if(!runFlag) {
        m_deletePtr = true;
        m_running = new bool(true);
    } else {
        m_running = runFlag;
    }

    for(unsigned i = 0; i < m_numThreads; i++) {
        m_threads.emplace_back([this] {Execute();});
        m_idMap.emplace(m_threads.crbegin()->get_id(), i);
    }
    return true;
}

/**
 * @brief Waits for all threads to complete before returning
 *
 * @return true if all threads were able to join
 */
bool MultiThread::Join()
{
    std::unique_lock<std::mutex> guard (m_threadMutex);
    auto deleteThreads = std::move(m_threads);
    guard.unlock();

    bool rc = true;
    for(auto&& t: deleteThreads) {
        try {
            t.join();
        } catch (const std::system_error& e) {
            if(e.code() == std::errc::resource_deadlock_would_occur) {
                std::cerr << "Warning: Cant join yourself! detaching..." << std::endl;
                try {
                    t.detach();
                } catch (const std::system_error& e) {
                    std::cerr << "Warning: Join could not detach: " << e.what() << std::endl;
                    rc = false;
                }
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

    guard.lock();
    if(m_deletePtr && m_running) {
        delete m_running;
        m_running = nullptr;
    }
    m_deletePtr = false;

    m_idMap.clear();

    return rc;
}

/**
 * @brief Waits for all threads to complete before returning
 *
 * @return true if all threads were able to join
 */
bool MultiThread::Detach()
{
    bool rc = true;
    for(auto&& t: m_threads) {
        if(!t.joinable() || t.get_id() == std::this_thread::get_id()) {
            rc = false;
            continue;
        }
        t.detach();
    }

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
