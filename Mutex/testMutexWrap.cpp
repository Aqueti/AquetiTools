/**
*  \file testMutexWrap.cpp
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

#include <mutex>
#include "AclMutexWrap.h"
#include "shared_mutex.h"

using namespace acl;

int test()
{

	std::cout << "testing std::mutex wrap" << std::endl;
	AclMutex m;
	m.lock();
	m.unlock();
	std::cout << std::endl << std::endl;

	std::cout << "testing aqueti shared mutex wrap" << std::endl;
	std::cout << "only does anything if built with -DDEBUG_CACHE" << std::endl;
	shared_mutex s;
	s.lock();
	s.unlock();
	std::cout << std::endl << std::endl;


	std::cout << "testing recursive mutex wrap" << std::endl;
	AclRecursiveMutex r;
	r.lock();
	r.unlock();
	std::cout << std::endl << std::endl;


	{
		std::cout << "testing lock guard on std::mutex wrap" << std::endl;
		std::lock_guard<AclMutex> g(m);
		std::cout << std::endl << std::endl;
	}
	{
		std::cout << "testing lock guard on recursiv mutex wrap" << std::endl;
		std::lock_guard<AclRecursiveMutex> g(r);
		std::cout << "lock guard recursive" << std::endl;
		std::cout << std::endl << std::endl;
	}

	{ 
		std::cout << "testing aqueti shared lock on shared mutex wrap" << std::endl;
		std::cout << "only does anything if built with -DDEBUG_CACHE" << std::endl;
		shared_lock sl(s);
		sl.unlock();
		sl.lock();
		std::cout << std::endl << std::endl;
	}

	return 0;

}

int test1()
{
	test();
	return 0;
}

int main(int arg, char **argv)
{
	std::cout << "testing mutex wrap" << std::endl;

	test1();

}
