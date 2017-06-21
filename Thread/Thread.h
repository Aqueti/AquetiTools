//==============================================================================
// A simple C++ wrapper class for c++11 threads
//
// The intended usage of this class is via inheritence--it does nothing itself.
// First, create a new class that inherits from Thread. Overide the Setup and
// Execute methods. The Setup method is intended to be executed once. The
// Execute method is the actual thread (it contains the thread's execution
// loop, and does not exit except under special (i.e. fault) circumstances).
//==============================================================================
#pragma once
#include <thread>
#include <mutex>

namespace atl
{
//************************************************************
/**
 *!\brief A simple C++ wrapper class for POSIX threads.
 *
 * The intended usage of this class is via inheritence--it does nothing itself.
 * First, create a new class that inherits from jthread. Overide the Setup and
 * Execute methods. The Setup method is intended to be executed once. The
 * Execute method is the actual thread (it contains the thread's execution
 * loop, and does not exit except under special (i.e. fault) circumstances).
 **/
//************************************************************
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

bool testThread(bool printFlag = true);
}
