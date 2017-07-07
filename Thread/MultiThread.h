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
    MultiThread(int numThreads=2): m_numThreads(numThreads) {}
    virtual ~MultiThread();
    virtual bool setNumThreads(unsigned);
    virtual bool Start(bool* runFlag = nullptr);
    virtual bool Join();
    virtual bool Detach();

protected:
    virtual int getMyId();

    unsigned                    m_numThreads;   //<! Number of threads to spawn
    std::vector<std::thread>    m_threads;      //<! Running threads
    std::map<std::thread::id,int>  m_idMap;      //<! map of Ids
};
}
