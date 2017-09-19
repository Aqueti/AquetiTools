/**
 * \file Thread.cpp
 **/

#include <iostream>
#include <sys/types.h>
#include <chrono>
#include <thread>
#include <vector>
#include <Timer.h>
#include <fstream>
#include "JsonBox.h"

#include "Thread.h"

std::ofstream threadTest;

namespace atl
{

/**
 * \brief  Destructor
 *
 * This destructor waits for the internal thread to complete (join)
 * before self-destructing.
 **/
Thread::~Thread()
{
    Stop();
    Thread::Join();
}

/**
 * \brief Function called to start thread execution
 *
 * \param [in] runFlag boolean pointer to terminate thread execution
 * \return std::thread object for this threads
 **/
bool Thread::Start( bool* runFlag )
{
    std::unique_lock<std::mutex> guard (m_threadMutex);
    if( (m_running && *m_running) || m_threadObj.joinable() ) {
        std::cerr << "WARNING: Thread threadObject already running" << std::endl;
        return false;
    }

    if(!runFlag) {
        m_deletePtr = true;
        m_running = new bool(true);
    } else {
        m_running = runFlag;
    }

    m_threadObj = std::thread([this] {Execute();});
    return true;
}

/**
 * \brief Execution function that starts thread processing
 *
 * This function is entry point into thread operation. This
 * function exits when the runPtr or running flag are set to false.
 **/
void Thread::Execute()
{
    while(isRunning()) {
        mainLoop();
    }
}

/**
 * \brief Main execution loop for the thread
 *
 * This function is the main processing loop. It needs to be overwritten for
 * each class instantiation. In the base class, it sleep for 1000uSecs
 **/
void Thread::mainLoop(void)
{
    threadTest << m_threadObj.get_id() <<": Thread mainLoop"<<std::endl;
    std::cerr << "WARNING: thread mainLoop method not overridden" << std::endl;
    sleep(1);
}

/**
 * \brief Waits for the thread to complete before returning
 *
 * This call is implements the std::thread::join for the class
 **/
bool Thread::Detach()
{
    try {
        m_threadObj.detach();
    } catch (const std::system_error& e) {
        std::cerr << "Warning: Join could not detach: " << e.what() << std::endl;
        return false;
    }
    return true;
}

/**
 * \brief Waits for the thread to complete before returning
 *
 * This call is implements the std::thread::join for the class
 **/
bool Thread::Join()
{
    bool rc;
    try {
        m_threadObj.join();
    } catch (const std::system_error& e) {
        if(e.code() == std::errc::resource_deadlock_would_occur) {
            std::cerr << "Warning: Can't join yourself! detaching..." << std::endl;
            /*try {
                m_threadObj.detach();
            } catch (const std::system_error& e) {
                std::cerr << "Warning: Join could not detach: " << e.what() << std::endl;
                rc = false;
            }*/
        } else if(e.code() == std::errc::no_such_process) {
            std::cerr << "Warning: Thread not valid: " << e.what() << std::endl;
            rc = false;
        } else if(e.code() == std::errc::invalid_argument) {
            // this just means the thread isn't running
            //std::cerr << "Warning: Thread not joinable: " << e.what() << std::endl;
            rc = false;
        } else {
            std::cerr << "Warning: Unknown thread error occurred: " << e.what() << std::endl;
            rc = false;
        }
    }

    std::unique_lock<std::mutex> guard (m_threadMutex);
    if(m_deletePtr && m_running) {
        delete m_running;
        m_running = nullptr;
    }
    m_deletePtr = false;

    return rc;
}

/**
 * \brief forces the thread to stop running
 **/
void Thread::Stop()
{
    std::lock_guard<std::mutex> guard (m_threadMutex);
    if(m_running) {
        *m_running = false;
    }
}

/**
 * @brief Returns true if thread is currently running
 */
bool Thread::isRunning()
{
    std::lock_guard<std::mutex> guard (m_threadMutex);
    if(m_running) {
        return *m_running;
    } else {
        return false;
    }
}
}

