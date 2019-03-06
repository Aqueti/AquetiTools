/******************************************************************************
 *
 * \file ThreadWorker.h
 * \brief 
 * \author Andrew Ferg
 *
 * Copyright Aqueti 2019
 *
 *****************************************************************************/
#pragma once

#include <mutex>
#include "Thread.h"

namespace atl {

class ThreadWorker : private Thread
{
    public: 
        ThreadWorker(std::function<void()> f=nullptr);
        ~ThreadWorker();
        void setMainLoopFunction(std::function<void()> f=nullptr);

        using Thread::Start;
        using Thread::Stop;
        using Thread::Join;
        using Thread::isRunning;

    private:
        void mainLoop();

        std::mutex m_mainLoopMutex;
        std::function<void()> m_mainLoopFunction = nullptr;
};

} //end namespace atl
