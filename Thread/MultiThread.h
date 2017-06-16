#pragma once

#include "Thread.h"
#include <vector>

namespace atl
{

class MultiThread : public Thread
{
public:
    MultiThread(int numThreads=2): m_numThreads(numThreads) {}
    virtual bool setNumThreads(unsigned);
    virtual bool Start(bool* runFlag = nullptr);
    virtual bool Join();

protected:
    unsigned                    m_numThreads;   //<! Number of threads to spawn
    std::vector<std::thread>    m_threads;      //<! Running threads
};

bool testMultiThread(bool printFlag = true);
}
