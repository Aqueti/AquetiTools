/******************************************************************************
 *
 * \file shared_mutex.h
 *
 *****************************************************************************/

#pragma once

//#include <condition_variable>
#include <mutex>
#include <atomic>

namespace atl
{
    class shared_mutex
    {
        private:
            std::mutex m_read;
            std::mutex m_mutex;
            std::atomic_int readers;

        public:
            shared_mutex();
            shared_mutex( const shared_mutex& other ) = delete;
            shared_mutex& operator=( const shared_mutex& ) = delete;
            void lock();
            bool try_lock();
            void unlock();
            void lock_shared();
            bool try_lock_shared();
            void unlock_shared();

    };

    class shared_lock
    {
        private:
            shared_mutex* m_mutex;
            std::atomic_bool owns;

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

