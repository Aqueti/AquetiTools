/******************************************************************************
 *
 * \file shared_mutex.cpp
 *
 *****************************************************************************/

#include "shared_mutex.h"

namespace atl
{
    /////////////////////////////////
    // shared_mutex implementation //
    /////////////////////////////////

    //shared_mutex::~shared_mutex()
    //{
        //writer = false;
        //readers = 0;
        //m_cv.notify_all();
        //m_read.unlock();
        //m_mutex.unlock();
    //}

    shared_mutex::shared_mutex()
    {
      readers = 0;
    }

    void shared_mutex::lock()
    {
        m_mutex.lock();

        //std::unique_lock<std::mutex> lock( m_mutex );
        //m_cv.wait( lock, [&]{ return !writer && !readers; } );
        //writer = true;
    }

    bool shared_mutex::try_lock()
    {
        return m_mutex.try_lock();
    }

    void shared_mutex::unlock()
    {
        m_mutex.unlock();

        //std::unique_lock<std::mutex> lock( m_mutex );
        //writer = false;
        //m_cv.notify_all();
    }

    void shared_mutex::lock_shared()
    {
        m_read.lock();
        readers++;
        if( readers == 1 ) m_mutex.lock();
        m_read.unlock();

        //std::unique_lock<std::mutex> lock( m_mutex );
        //m_cv.wait( lock, [&]{ return !writer; } );
        //readers++;
    }

    bool shared_mutex::try_lock_shared()
    {
        bool rc = true;
        m_read.lock();
        readers++;
        if( readers == 1 ){
            rc = m_mutex.try_lock();
            if( !rc ){
                readers--;
            }
        }
        m_read.unlock();
        return rc;
    }

    void shared_mutex::unlock_shared()
    {
        m_read.lock();
        readers--;
        if( !readers ) m_mutex.unlock();
        m_read.unlock();

        //std::unique_lock<std::mutex> lock( m_mutex );
        //readers--;
        //m_cv.notify_all();
    }


    ////////////////////////////////
    // shared_lock implementation //
    ////////////////////////////////

    shared_lock::shared_lock( shared_mutex& m )
    {
        owns = true;
        m_mutex = &m;
        m_mutex->lock_shared();
    }

    shared_lock::shared_lock( shared_mutex& m, std::try_to_lock_t t )
    {
        owns = false;
        m_mutex = &m;
        if( m_mutex->try_lock_shared() ){
            owns = true;
        }
    }

    shared_lock::shared_lock( shared_mutex& m, std::defer_lock_t t )
    {
        owns = false;
        m_mutex = &m;
    }

    shared_lock::shared_lock( shared_mutex& m, std::adopt_lock_t t )
    {
        owns = true;
        m_mutex = &m;
    }

    shared_lock::~shared_lock()
    {
        if( owns ) m_mutex->unlock_shared();
    }

    void shared_lock::lock()
    {
        m_mutex->lock_shared();
        owns = true;
    }

    bool shared_lock::try_lock()
    {
        if( m_mutex->try_lock_shared() ){
            owns = true;
            return true;
        }
        return false;
    }

    void shared_lock::unlock()
    {
        owns = false;
        m_mutex->unlock_shared();
    }

    bool shared_lock::owns_lock() const
    {
        return owns;
    }
}
