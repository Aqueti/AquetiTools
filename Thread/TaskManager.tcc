#include <map>
#include <thread>
#include <future>

#pragma once

namespace atl
{

template<typename Key, typename ReturnType>
class TaskManager
{
public:
    ReturnType performJob(Key id, std::function<ReturnType(void)> f);

protected:
    std::map<Key, std::shared_future<ReturnType>> m_futureMap;
    std::mutex m_mutex;
};

template<typename Key, typename ReturnType>
ReturnType TaskManager<Key, ReturnType>::performJob(Key id, std::function<ReturnType(void)> f)
{
    std::shared_future<ReturnType> fut;
    bool added;

    std::unique_lock<std::mutex> l(m_mutex);
    if(m_futureMap.find(id) == m_futureMap.end()) {
        fut = std::async(std::launch::deferred, f).share();
        m_futureMap[id] = fut;
        added = true;

    } else {
        fut = m_futureMap[id];
        added = false;
    }
    l.unlock();

    fut.wait();

    if(added) {
        l.lock();
        m_futureMap.erase(id);
        l.unlock();
    }

    return fut.get();
}

}
