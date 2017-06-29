/**
 * \file shared_mutex.cpp
 **/

#include "shared_mutex.h"

namespace atl
{

    //shared_mutex::~shared_mutex()
    //{
        //writer = false;
        //readers = 0;
        //m_cv.notify_all();
        //m_read.unlock();
        //m_mutex.unlock();
    //}

    /**
     * Constructor
     */
    shared_mutex::shared_mutex()
    {
      readers = 0;
    }

    /**
     * Locks a thread
     */
    void shared_mutex::lock()
    {
        m_mutex.lock();

        //std::unique_lock<std::mutex> lock( m_mutex );
        //m_cv.wait( lock, [&]{ return !writer && !readers; } );
        //writer = true;
    }

    /**
     * Checks if a lock is in place
     * 
     * @return Returns false if there is a lock
     */
    bool shared_mutex::try_lock()
    {
        return m_mutex.try_lock();
    }

    /**
     * Unlocks a thread
     */
    void shared_mutex::unlock()
    {
        m_mutex.unlock();

        //std::unique_lock<std::mutex> lock( m_mutex );
        //writer = false;
        //m_cv.notify_all();
    }

    /**
     * Checks if a lock is in place
     */
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

    /**
     * Checks if a lock is in place
     * 
     * @return Returns false if there is a lock
     */
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

    /**
     * Unlocks a shared thread
     */
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

    /**
     * Constructor
     */
    shared_lock::shared_lock( shared_mutex& m )
    {
        owns = true;
        m_mutex = &m;
        m_mutex->lock_shared();
    }

    /**
     * Constructor
     */
    shared_lock::shared_lock( shared_mutex& m, std::try_to_lock_t t )
    {
        owns = false;
        m_mutex = &m;
        if( m_mutex->try_lock_shared() ){
            owns = true;
        }
    }

    /**
     * Constructor
     */
    shared_lock::shared_lock( shared_mutex& m, std::defer_lock_t t )
    {
        owns = false;
        m_mutex = &m;
    }

    /**
     * Constructor
     */
    shared_lock::shared_lock( shared_mutex& m, std::adopt_lock_t t )
    {
        owns = true;
        m_mutex = &m;
    }

    /**
     * Destructor
     */
    shared_lock::~shared_lock()
    {
        if( owns ) m_mutex->unlock_shared();
    }

    /**
     * Locks a thread
     */
    void shared_lock::lock()
    {
        m_mutex->lock_shared();
        owns = true;
    }

    /**
     * Checks if a lock is in place
     * 
     * @return Returns false if there is a lock
     */
    bool shared_lock::try_lock()
    {
        if( m_mutex->try_lock_shared() ){
            owns = true;
            return true;
        }
        return false;
    }

    /**
     * Unlocks a thread
     */
    void shared_lock::unlock()
    {
        owns = false;
        m_mutex->unlock_shared();
    }

    /**
     * Checks if the caller owns the lock on a thread
     * 
     * @return Returns a boolean on whether the lock is owned
     */
    bool shared_lock::owns_lock() const
    {
        return owns;
    }
}
