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
#include "AtlMutexWrap.h"
#include "shared_mutex.h"

using namespace atl;

int main(int arg, char **argv)
{

	AtlMutex m;
	m.lock();

	m.unlock();

	shared_mutex s;
	s.lock();

	s.unlock();


	AtlRecursiveMutex r;


	r.lock();

	r.unlock();


	{
		std::lock_guard<AtlMutex> g(m);
		std::cout << "lock guard mutex" << std::endl;
	}
	{
		std::lock_guard<AtlRecursiveMutex> g(r);
		std::cout << "lock guard recursive" << std::endl;
	}

	{ 
		shared_lock sl(s);
		std::cout << "shared lock" << std::endl;
		sl.unlock();
		sl.lock();
	}

}
