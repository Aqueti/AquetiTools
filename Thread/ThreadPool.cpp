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
