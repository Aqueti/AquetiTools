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
protected:
    std::mutex    m_threadMutex;         //!< Mutex to signal closure
    std::thread   m_threadObj;           //!< Thread variable
    bool*         m_running = nullptr;   //!< Flag to stop running by a shared pointer
    bool          m_deletePtr = false;   //!< If true, delete pointer on destruction

    virtual void  Execute(void);
    virtual void  mainLoop(void);

public:
    virtual       ~Thread();

    virtual bool  Start(bool* runFlag=NULL );
    virtual void  Stop(void);
    virtual bool  Join(void);
    virtual bool  Detach(void);
    virtual bool  isRunning(void);
};
}
