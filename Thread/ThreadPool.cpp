#include "ThreadPool.h"

namespace atl
{

ThreadPool::ThreadPool(int numThreads, int maxJobLength, double timeout): MultiThread(numThreads), TSQueue<std::function<void()>>()
{
    set_max_size(maxJobLength);
    m_timeout = timeout;
}

bool ThreadPool::push_job(std::function<void()> f)
{
    return enqueue(f);
}

void ThreadPool::mainLoop()
{
    std::function<void()> f;
    if(dequeue(f, m_timeout) && f) {
        f();
    }
}

void ThreadPool::setTimeout(double timeout)
{
    m_timeout = timeout;
}

}
