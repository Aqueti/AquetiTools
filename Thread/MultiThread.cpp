//   Copyright Aqueti 2016.

// Distributed under the Boost Software License, Version 1.0.

//    (See accompanying file LICENSE_1_0.txt or copy at

//   http://www.boost.org/LICENSE_1_0.txt)

#include "MultiThread.h"
#include <iostream>
#include <fstream>

namespace atl
{

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
    if (m_running && *m_running) {
        std::cout << "WARNING: Threads already running." << std::endl;
        return false;
    }

    if (!runFlag) {
        m_deletePtr = true;
        m_running = new bool(true);
    } else {
        m_running = runFlag;
    }

    for (unsigned i = 0; i < m_numThreads; i++) {
        m_threads.emplace(m_threads.end(), [this] {Execute();});
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

    bool rc = true;
    for (auto&& t: m_threads) {
        if (!t.joinable() || t.get_id() == std::this_thread::get_id()) {
            std::cout << "WARNING: Thread id: " << t.get_id() << " not joinable" << std::endl;
            rc = false;
            continue;
        }
        t.join();
    }

    std::lock_guard<std::mutex> guard (m_threadMutex);
    m_threads.clear();

    if (m_deletePtr && m_running) {
        delete m_running;
        m_running = nullptr;
    }
    m_deletePtr = false;

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
* \brief Test function
* 
* \param [in] printFlag boolean (true prints info to console)
* \return true if the test succeeded
**/
JsonBox::Value testMultiThread(bool printFlag)
{
    JsonBox::Value resultString;

    bool rc = true;
    MultiThread mThread(5);

    if (printFlag) {
        std::cout << "Test Stop function" << std::endl;
    }

    mThread.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    mThread.Stop();
    mThread.Join();

    if (printFlag) {
        std::cout << "Test running flag" << std::endl;
    }

    static bool running = true;
    mThread.Start(&running);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if (printFlag) {
        std::cout << "Running being set to false!" << std::endl;
    }

    running = false;
    rc = mThread.Join();

    if (!rc) {
        if (printFlag) {
            std::cout << "Unable to join thread!" << std::endl;
            std::cout << "Thread test error. See ThreadTest.log" << std::endl;
        }
        resultString["Join"] = "fail";
        resultString["pass"] = false;
        return resultString;
    }
    resultString["Join"] = "pass";

    if (printFlag) {
        std::cout << "Joined" << std::endl;
    }

    //Vector tests (timed)
    size_t threadCount = 50;
    running = true;
    std::vector<Thread> threadVect(threadCount);

    resultString["pass"] = true;
    return resultString;
}
}
