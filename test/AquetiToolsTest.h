
#pragma once

// includes
#include "JsonBox.h"
#include <Timer.h>
#include <Thread.h>
#include <MultiThread.h>
#include <ThreadPool.h>
//#include <LruCache.cpp>
//#include <TSMap.cpp>
#include <TSMap.tcc>
#include <aquetitools/revision.h>
#include <iostream>
#include <fstream>
#include <string.h>
//#include <TSQueue.cpp>
#include <TSQueue.tcc>


#include "ThreadPool.h"
#include <CRC.hpp>

/**
 * Perform unit tests for the aquetitools repository
 *
 * @param unitList The list of units to run tests on.
 * @param testSubmodules The boolean, if true run unit tests on submodules as well.
 * @param valgrind The boolean, if true run with valgrind settings.
 * @return JsonBox value with results
 */
JsonBox::Value testAquetiTools(std::vector<std::string> unitList = {"Timer", "CRC", 
	"Thread", "MultiThread", "ThreadPool", "LruCache", "TSMap", "TSQueue"}, 
	bool testSubmodules = true, bool valgrind = false);
