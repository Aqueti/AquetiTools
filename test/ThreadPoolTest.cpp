/**
 * \file ThreadPoolTest.cpp
 **/

#include "AquetiToolsTest.h"

namespace atl {
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

    for (int j = 0; j < 1000; j++) {
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
JsonBox::Value testThreadPool()
{
    JsonBox::Value resultString; //!< Brief JsonBox value with unit test results

    bool ret = doThreadPoolThing(5);
    ret = doThreadPoolThing(1) && ret;

    if (!ret) {
        std::cout << "Threadpool test failed." << std::endl;
        resultString["pass"] = false;
        return resultString;
    }
    resultString["pass"] = true;
    return resultString;
}
}