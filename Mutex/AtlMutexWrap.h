/**
*  \file AtlMutexWrap.h
*  \brief 
*  \author Drew Perkins
*  \version 
*  \date 2019-06-17
*  
*  *  Copyright (C) 
*  2019 - Aqueti Inc
*  Aqueti Inc. Proprietary 
*  3333 Durham Chapel Hill Blvd Suite D100, Durham, North Carolina 27704 
**/  


#include <thread>
#include <iostream>
#include <mutex>


#pragma once

namespace atl {




/**
*  \brief mutex wrapper to print when acquired and released
*  does nothing if DEBUG not defined
**/
template <typename T> 
class MutexWrap : public T
{
	
#ifdef DEBUG 
public:

	/**
	*  \brief wraps lock
	**/
	void lock()
	{
		std::thread::id tid = std::this_thread::get_id();
		std::cout << "thread " << tid << " attempting to acquire mutex " << (int64_t)this << std::endl;
		T::lock();
		std::cout << "thread " << tid << " acquired mutex " << (int64_t)this << std::endl;
	}

	/**
	*  \brief wraps unlock
	**/
	void unlock()
	{
		std::thread::id tid = std::this_thread::get_id();
		T::unlock();
		std::cout << "thread " << tid << " released mutex " << (int64_t)this << std::endl;
	}

	/**
	*  \brief wraps try_lock
	**/
	bool try_lock()
	{
		std::thread::id tid = std::this_thread::get_id();
		std::cout << "thread " << tid << " trying mutex " << (int64_t)this << std::endl;
		bool rv = T::try_lock();
		if(rv){
			std::cout << "thread " << tid << " acquired mutex " << (int64_t)this << std::endl;
		} else {
			std::cout << "thread " << tid << " did not acquire mutex " << (int64_t)this << std::endl;
		}
		return rv;
	}

	MutexWrap(){
		std::cout << "constructing atl mutex wrap " << (uint64_t)this << std::endl;
	}

	~MutexWrap(){
		std::cout << "destroying atl mutex wrap " << (uint64_t)this << std::endl;
	}

#endif
};



typedef MutexWrap<std::recursive_mutex> AtlRecursiveMutex;
typedef MutexWrap<std::mutex> AtlMutex;

} // namespace atl


