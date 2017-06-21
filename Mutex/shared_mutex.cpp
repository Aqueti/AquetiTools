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

    /**
    * \brief constructor for the shared_mutex class
    **/
    shared_mutex::shared_mutex()
    {
        readers = 0;
    }

    /** 
    * \brief locks the mutex
    **/
    void shared_mutex::lock()
    {
        m_mutex.lock();

        //std::unique_lock<std::mutex> lock( m_mutex );
        //m_cv.wait( lock, [&]{ return !writer && !readers; } );
        //writer = true;
    }

    /**
    * \brief tries to lock the mutex
    *
    * \return true if the mutex is successfully locked
    **/
    bool shared_mutex::try_lock()
    {
        return m_mutex.try_lock();
    }

    /**
    * \brief unlocks the mutex
    **/
    void shared_mutex::unlock()
    {
        m_mutex.unlock();

        //std::unique_lock<std::mutex> lock( m_mutex );
        //writer = false;
        //m_cv.notify_all();
    }

    /**
    * \brief locks the mutex if it is shared
    **/
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
    * \brief tries to lock shared mutex
    *
    * \return true if the shared mutex is successfully locked
    **/
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
    * \brief unlocks shared mutex
    **/
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
    * \brief initializes shared mutex
    *
    * \param [in] m reference to shared mutex
    **/
    shared_lock::shared_lock(shared_mutex& m)
    {
        owns = true;
        m_mutex = &m;
        m_mutex->lock_shared();
    }

    /**
    * \brief tries to lock the shared mutex
    * 
    * \param [in] m reference to shared mutex
    * \param [in] t try to lock mutex without blocking
    **/
    shared_lock::shared_lock(shared_mutex& m, std::try_to_lock_t t)
    {
        owns = false;
        m_mutex = &m;
        if(m_mutex->try_lock_shared()) {
            owns = true;
        }
    }

    /**
    * \brief does not acquire ownership of mutex
    * 
    * \param [in] m reference to shared mutex
    * \param [in] t do not lock the mutex
    **/
    shared_lock::shared_lock(shared_mutex& m, std::defer_lock_t t)
    {
        owns = false;
        m_mutex = &m;
    }

    /** 
    * \brief assume the calling thread already has ownership of mutex
    * 
    * \param [in] m reference to shared mutex
    * \param [in] t assume already locked
    **/
    shared_lock::shared_lock(shared_mutex& m, std::adopt_lock_t t)
    {
        owns = true;
        m_mutex = &m;
    }

    /**
    * \brief destructor for the shared_lock class
    **/
    shared_lock::~shared_lock()
    {
        if(owns) m_mutex->unlock_shared();
    }

    /**
    * \brief locks the shared mutex
    **/
    void shared_lock::lock()
    {
        m_mutex->lock_shared();
        owns = true;
    }

    /**
    * \brief tries to lock the shared mutex
    * 
    * \return true if lock is successful
    **/
    bool shared_lock::try_lock()
    {
        if( m_mutex->try_lock_shared() ){
            owns = true;
            return true;
        }
        return false;
    }

    /**
    * \brief unlocks the shared mutex
    **/
    void shared_lock::unlock()
    {
        owns = false;
        m_mutex->unlock_shared();
    }

    /**
    * \brief gets the value of the owns boolean
    *
    * \return true if owns is true
    **/
    bool shared_lock::owns_lock() const
    {
        return owns;
    }
}
