/**
 * \file ThreadPool.cpp
 **/

#include "ThreadPool.h"

namespace acl
{

/**
* \brief initializes the thread pool
*
* \param [in] numThreads the number of threads
* \param [in] maxJobLength the maximum number of jobs that can be submitted
* \param [in] timeout the time a thread should process before moving on
**/
ThreadPool::ThreadPool(int numThreads, int maxJobLength, double timeout): MultiThread(numThreads), TSQueue<std::function<void()>>()
{
    set_max_size(maxJobLength);
    m_timeout = timeout;
}

/**
* \brief adds jobs to the pool
*
* \param [in] f the job to be added
* \return true if the job has been successfully enqueued
**/
bool ThreadPool::push_job(std::function<void()> f)
{
    return enqueue(f);
}

/**
* \brief main function to loop through the queue and run jobs
**/
void ThreadPool::mainLoop()
{
    std::function<void()> f;
    if (dequeue(f, m_timeout) && f) {
        f();
    }
}

/**
* \brief sets the timeout value
*
* \param [in] timeout the timeout value to be set
**/
void ThreadPool::setTimeout(double timeout)
{
    m_timeout = timeout;
}
}
