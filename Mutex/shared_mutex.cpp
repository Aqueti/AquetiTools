/**
 * \file shared_mutex.cpp
 **/

#include "shared_mutex.h"

#ifdef USE_HELGRIND
#include "helgrind.h"
#endif //USE_HELGRIND

namespace atl
{

    shared_mutex::~shared_mutex()
    {
#ifdef USE_HELGRIND
        // Use Readers as unique ID so it's not a mutex
        ANNOTATE_RWLOCK_DESTROY(this);
#endif //USE_HELGRIND
    }

    /**
     * Constructor
     */
    shared_mutex::shared_mutex(): readers(0)
    {
#ifdef USE_HELGRIND
        // Due to virtual destructor, this should be a VTable pointer
        ANNOTATE_RWLOCK_CREATE(this);
#endif //USE_HELGRIND
    }

    /**
     * Locks a thread
     */
    void shared_mutex::lock()
    {
        m_mutex.lock();
#ifdef USE_HELGRIND
        ANNOTATE_RWLOCK_ACQUIRED(this, 1);
#endif //USE_HELGRIND
    }

    /**
     * Checks if a lock is in place
     * 
     * @return Returns false if there is a lock
     */
    bool shared_mutex::try_lock()
    {
        bool locked = m_mutex.try_lock();
#ifdef USE_HELGRIND
        if(locked) {
            ANNOTATE_RWLOCK_ACQUIRED(this, 1);
        }
#endif //USE_HELGRIND
        return locked;
    }

    /**
     * Unlocks a thread
     */
    void shared_mutex::unlock()
    {
#ifdef USE_HELGRIND
        ANNOTATE_RWLOCK_RELEASED(this, 1);
#endif //USE_HELGRIND
        m_mutex.unlock();
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
#ifdef USE_HELGRIND
        ANNOTATE_RWLOCK_ACQUIRED(this, 0);
#endif //USE_HELGRIND
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
#ifdef USE_HELGRIND
        if(rc) {
            ANNOTATE_RWLOCK_ACQUIRED(this, 0);
        }
#endif //USE_HELGRIND
        return rc;
    }

    /**
     * Unlocks a shared thread
     */
    void shared_mutex::unlock_shared()
    {
#ifdef USE_HELGRIND
        ANNOTATE_RWLOCK_RELEASED(this, 0);
#endif //USE_HELGRIND
        m_read.lock();
        readers--;
        if( !readers ) m_mutex.unlock();
        m_read.unlock();
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
