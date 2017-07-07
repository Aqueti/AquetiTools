/**
 * \file MultiThread.h
 **/

#pragma once

#include "Thread.h"
#include "JsonBox.h"
#include <vector>

namespace atl
{

/**
* \brief Multithread class that calls on Thread class functions
**/
class MultiThread : public Thread
{
public:
    MultiThread(int numThreads = 2): m_numThreads(numThreads) {}	//!< Number of threads
    virtual ~MultiThread();
    virtual bool setNumThreads(unsigned numThreads);
    virtual bool Start(bool* runFlag = nullptr);
    virtual bool Join();
    virtual bool Detach();
    virtual int getMyId();

private:
    unsigned                    m_numThreads;   //!< Number of threads to spawn
    std::vector<std::thread>    m_threads;      //!< Running threads
    std::map<std::thread::id,int> m_idMap; 		//<! map of Ids
};
}
