//   Copyright Aqueti 2016.

// Distributed under the Boost Software License, Version 1.0.

//    (See accompanying file LICENSE_1_0.txt or copy at

//   http://www.boost.org/LICENSE_1_0.txt)

//******************************************************************************
// A simple C++ wrapper class for C++11 threads
//
//******************************************************************************

#include <iostream>
#include <sys/types.h>
#include <chrono>
#include <thread>
#include <vector>
#include <Timer.h>
#include <fstream>

#include "Thread.h"

std::ofstream threadTest;

namespace aqt
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
    Join();
}

/**
 * \brief Function called to start thread execution
 *
 * \param [in] runFlag boolean pointer to terminate thread execution
 * \return std::thread object for this threads
 **/
bool Thread::Start( bool* runFlag )
{
    if( (m_running && *m_running) || m_threadObj.joinable() ) {
        std::cout << "WARNING: Thread threadObject already running" << std::endl;
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
bool Thread::Join()
{
    if( !m_threadObj.joinable() || m_threadObj.get_id() == std::this_thread::get_id()) { //Don't join yourself!
        return false;
    }

    m_threadObj.join();

    std::unique_lock<std::mutex> guard (m_threadMutex);
    if(m_deletePtr && m_running) {
        delete m_running;
        m_running = nullptr;
    }
    m_deletePtr = false;

    return true;
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

/**
 * \brief Test function
 **/
bool testThread(bool printFlag)
{
    //create file to redirect output to
    threadTest.open("ThreadTest.log");

    bool rc = true;
    Thread cThread;

    if(printFlag) {
        threadTest << "Test Stop function"<<std::endl;
    }
    cThread.Start(NULL);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    cThread.Stop();
    cThread.Join();

    if(printFlag) {
        threadTest << "Test running flag"<<std::endl;
    }
    static bool running = true;
    cThread.Start(&running);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if(printFlag) {
        threadTest << "Running being set to false!" << std::endl;
    }
    running = false;
    rc = cThread.Join();
    if( !rc ) {
        if(printFlag) {
            threadTest << "Unable to join thread!"<<std::endl;
            std::cout << "Thread test error. See ThreadTest.log" << std::endl;
        }
    }
    if(printFlag) {
        threadTest << "Joined"<<std::endl;
    }

    //Vector tests (timed)
    size_t threadCount = 50;
    running = true;
    std::vector<Thread> threadVect(threadCount);

    if(printFlag) {
        threadTest << "Testing vector "<<std::endl << std::endl;
    }
    //Spawn threadCount threads
    for( uint16_t i = 0; i < threadCount; i++ ) {
        threadVect[i].Start(&running);
    }


    //Stop running
    running = false;

    //Join all
    for( uint16_t i = 0; i < threadCount; i++ ) {
        threadVect[i].Join();
    }

    //ReSpawn threadCount threads
    for( uint16_t i = 0; i < threadCount; i++ ) {
        threadVect[i].Start(&running);
    }

    //Stop threadCount threads
    for( unsigned int i = 0; i < threadCount; i++ ) {
        threadVect[i].Stop();
    }


    std::system("rm ThreadTest.log");
    return true;

}
}

