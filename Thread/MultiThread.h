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
    virtual bool setNumThreads(unsigned numThreads);
    virtual bool Start(bool* runFlag = nullptr);
    virtual bool Join();

private:
    unsigned                    m_numThreads;   //!< Number of threads to spawn
    std::vector<std::thread>    m_threads;      //!< Running threads
};

JsonBox::Value testMultiThread(bool printFlag = true);
}
