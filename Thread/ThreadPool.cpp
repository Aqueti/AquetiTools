#include "ThreadPool.h"

namespace atl
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
    if(dequeue(f, m_timeout) && f) {
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

/**
* \brief runs the actual thread pool
*
* \param [in] threads the number of threads to be run
* \return true if the number of threads run equals the number of threads entered
**/
bool doThreadPoolThing(int threads)
{
    std::cout << threads << " threads: ";
    atl::ThreadPool tp(threads, 1000);
    std::atomic_int i(0);
    auto fun = [&](){
        i++;
        atl::sleep(0.001);
    };

	atl::Timer t;
    tp.Start();

    for(int j = 0; j < 1000; j++) {
        tp.push_job(fun);
    }

    tp.wait_until_empty();
    std::cout << "elapsed time: " << t.elapsed() << std::endl;

    tp.Stop();
    tp.Join();
    return i == 1000;
}

/**
* \brief tests the thread pool
*
* \return true if the test passes
**/
bool testThreadPool()
{
    bool ret = doThreadPoolThing(5);
    ret = doThreadPoolThing(1) && ret;

    if(ret) {
        std::cout << "Threadpool test passed!" << std::endl;
    }
    return ret;
   	}

}
