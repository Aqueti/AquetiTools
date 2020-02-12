/**
 * \file Thread.h
 **/

#pragma once
#include <thread>
#include <mutex>
#include <atomic>

namespace acl
{

/**
 * @class Thread
 *
 * @brief A class for using threads
 */
class Thread
{
protected:
    std::thread         m_threadObj;    //!< Thread variable
    std::atomic_bool    m_running;      //!< Flag to stop running by a shared pointer

    virtual void  Execute(void);
    virtual void  mainLoop(void);

public:
                  Thread();
    virtual       ~Thread();

    virtual bool  Start(void);
    virtual void  Stop(void);
    virtual bool  Join(void);
    virtual bool  Detach(void);
    virtual bool  isRunning(void);
};
}
