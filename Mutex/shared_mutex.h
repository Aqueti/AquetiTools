/**
 * \file shared_mutex.h
 **/

#pragma once

#include <mutex>
#include <atomic>
#include "AtlMutexWrap.h"

namespace atl
{
    /**
     * @class shared_mutex
     *
     * @brief A class that allows threaded applications to lock threads and prevent deadlocks and race conditions
     */
    class shared_mutex
    {
        private:
            //std::mutex m_read; //!< Brief readers
			AtlMutex m_read;
       //     std::mutex m_mutex;  //!< Brief member fields for mutext
			AtlMutex m_mutex;
            std::atomic_int readers; //!< Brief the number of readers

        public:
            shared_mutex();
            virtual ~shared_mutex();
            shared_mutex( const shared_mutex& other ) = delete;
            shared_mutex& operator=( const shared_mutex& ) = delete;
            void lock(); 
            bool try_lock();
            void unlock();
            void lock_shared();
            bool try_lock_shared();
            void unlock_shared();

    };

    /**
     * @class shared_lock
     *
     * @brief A class that allows shared locking of threads
     */
    class shared_lock
    {
        private:
            shared_mutex* m_mutex; //!< Brief pointer to readers
            std::atomic_bool owns; //!< Brief boolean for whether the lock is owned

        public:
            shared_lock( shared_mutex& m );
            shared_lock( shared_mutex& m, std::try_to_lock_t t );
            shared_lock( shared_mutex& m, std::defer_lock_t t );
            shared_lock( shared_mutex& m, std::adopt_lock_t t );
            shared_lock( const shared_lock& other ) = delete;
            ~shared_lock();
            void lock();
            bool try_lock();
            void unlock();
            bool owns_lock() const;
    };

}

