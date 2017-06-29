
#pragma once

#include "JsonBox.h"
#include <Timer.h>
#include <Thread.h>
#include <MultiThread.h>
#include <ThreadPool.h>
#include <LruCache.tcc>
//#include <TSMap.cpp>
#include <TSMap.tcc>
#include <aquetitools/revision.h>
#include <iostream>
#include <fstream>
#include <string.h>
//#include <TSQueue.cpp>
#include <TSQueue.tcc>
#include "ThreadPool.h"
#include "TaskManager.tcc"
#include <CRC.hpp>
#include <assert.h>

namespace atl{

/**
 * Perform unit tests for the aquetitools repository
 *
 * @param unitList The list of units to run tests on.
 * @param testSubmodules The boolean, if true run unit tests on submodules as well.
 * @param valgrind The boolean, if true run with valgrind settings.
 * @return JsonBox value with results
 */
JsonBox::Value testAquetiTools(std::vector<std::string> unitList = {"Timer", "CRC", 
	"Thread", "MultiThread", "ThreadPool", "LruCache", "TSMap", "TSQueue", "TaskManager"}, 
	bool testSubmodules = true, bool printFlag = true, bool assertFlag = false, bool valgrind = false);

//Functional tests
JsonBox::Value testTimer(bool printFlag = true, bool assertFlag = false);
JsonBox::Value testThread(bool printFlag = true);
JsonBox::Value testMultiThread(bool printFlag = true);
JsonBox::Value testThreadPool();
JsonBox::Value testLruCache(unsigned int numThreads = 100, bool printFlag = true, bool assertFlag = false);
JsonBox::Value testTSQueue(unsigned int numThreads = 20, bool printFlag = true, bool assertFlag = false);
JsonBox::Value testCRC();
JsonBox::Value testTSMap(bool printFlag = false, bool assertFlag = false, bool valgrind = false);
JsonBox::Value testTaskManager(int threads = 1000, bool valgrind = false, bool printFlag = true, bool assertFlag = false);

//Timer test helper functions
bool sleepTest(JsonBox::Value& resultString, double delayTime, double sleepElapsed,
                double timeVariance, bool printFlag, bool assertFlag, int testno);
bool objectIDSizeTest(bool printFlag, bool assertFlag, JsonBox::Value& resultString);

//ThreadPool test helper functions
bool doThreadPoolThing(int threads);
}