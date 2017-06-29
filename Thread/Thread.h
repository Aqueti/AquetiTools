/**
 * \file Thread.h
 **/

#pragma once
#include <thread>
#include <mutex>
#include "JsonBox.h"

namespace atl
{

/**
 * @class Thread
 *
 * @brief A class for using threads
 */
class Thread
{
private:


public:
    virtual       ~Thread();

    virtual bool  Start(bool* runFlag=NULL );
    virtual void  Stop(void);
    virtual bool  Join(void);
    virtual bool  isRunning(void);

    virtual void  Execute(void);
    virtual void  mainLoop(void);

    bool*         m_running = nullptr;   //!< Flag to stop running by a shared pointer
    bool          m_deletePtr = false;   //!< If true, delete pointer on destruction
    std::mutex    m_threadMutex;         //!< Mutex to signal closure
    std::thread   m_threadObj;           //!< Thread variable
};
}
