#include "MultiThread.h"
#include "TSQueue.tcc"

#include <functional>
#include <atomic>

#pragma once

namespace aqt
{

class ThreadPool: public MultiThread, protected TSQueue<std::function<void()>>
{
public:
    ThreadPool(int numThreads = 1, int maxJobLength = 50, double timeout = 1);

    bool push_job(std::function<void()> f);
    void setTimeout(double timeout);

    using TSQueue<std::function<void()>>::size;
    using TSQueue<std::function<void()>>::delete_all;
    using TSQueue<std::function<void()>>::set_max_size;
    using TSQueue<std::function<void()>>::get_max_size;
    using TSQueue<std::function<void()>>::wait_until_empty;

protected:
    std::atomic<double> m_timeout;
    virtual void mainLoop();
};

}
