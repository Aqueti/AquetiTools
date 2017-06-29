/**
 * \file AquetiToolsTest.h
 **/

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
 * @return JsonBox value of the test results
 */
JsonBox::Value testAquetiTools(std::vector<std::string> unitList = {"Timer", "CRC", 
	"Thread", "MultiThread", "ThreadPool", "LruCache", "TSMap", "TSQueue", "TaskManager"} /**< List of units to test */, 
	bool testSubmodules = true /**< Boolean for testing submodules*/, bool printFlag = true /**< Boolean for printing out to console */, 
	bool assertFlag = false /**< Boolean for asserts in tests */, bool valgrind = false /**< Boolean for setting valgrind test */);

/**
 * Runs the tests for Timer
 *
 * @param printFlag A boolean, if true tests print out messages to the console
 * @param assertFlag A boolean, if true program halts on error
 * @return JsonBox value of the test results
 */
JsonBox::Value testTimer(bool printFlag = true, bool assertFlag = false);

/**
 * Runs the tests for Timer
 *
 * @param printFlag A boolean, if true tests print out messages to the console
 * @return JsonBox value of the test results
 */
JsonBox::Value testThread(bool printFlag = true);

/**
 * Runs the tests for Timer
 *
 * @param printFlag A boolean, if true tests print out messages to the console
 * @return JsonBox value of the test results
 */
JsonBox::Value testMultiThread(bool printFlag = true);

/**
 * Runs the tests for Timer
 *
 * @return JsonBox value of the test results
 */
JsonBox::Value testThreadPool();

/**
 * Runs the tests for Timer
 *
 * @param numThreads The number of threads used during unit testing (default 100)
 * @param printFlag A boolean, if true tests print out messages to the console
 * @param assertFlag A boolean, if true program halts on error
 * @return JsonBox value of the test results
 */
JsonBox::Value testLruCache(unsigned int numThreads = 100, bool printFlag = true, bool assertFlag = false);

/**
 * Runs the tests for Timer
 *
 * @param numThreads The number of threads used during unit testing (default 20)
 * @param printFlag A boolean, if true tests print out messages to the console
 * @param assertFlag A boolean, if true program halts on error
 * @return JsonBox value of the test results
 */
JsonBox::Value testTSQueue(unsigned int numThreads = 20, bool printFlag = true, bool assertFlag = false);

/**
 * Runs the tests for Timer
 *
 * @return JsonBox value of the test results
 */
JsonBox::Value testCRC();

/**
 * Runs the tests for Timer
 *
 * @param printFlag A boolean, if true tests print out messages to the console
 * @param assertFlag A boolean, if true program halts on error
 * @param valgrind A boolean, if true sets valgrind settings for unit testing
 * @return JsonBox value of the test results
 */
JsonBox::Value testTSMap(bool printFlag = false, bool assertFlag = false, bool valgrind = false);

/**
 * Runs the tests for Timer
 *
 * @param threads The number of threads used for testing (default 1000)
 * @param printFlag A boolean, if true tests print out messages to the console
 * @param assertFlag A boolean, if true program halts on error
 * @param valgrind A boolean, if true sets valgrind settings for unit testing
 * @return JsonBox value of the test results
 */
JsonBox::Value testTaskManager(int threads = 1000, bool printFlag = true, bool assertFlag = false, bool valgrind = false);

/**
 * Runs the tests for Timer
 *
 * @param resultString The JsonBox value that holds the results of the unit testing
 * @param delayTime A double that gives the delay time for sleeping
 * @param A double that shows the amount of time elapsed since the last sleep
 * @param A double 
 * @param printFlag A boolean, if true tests print out messages to the console
 * @param assertFlag A boolean, if true program halts on error
 * @return True if the sleeptest runs correctly
 */
bool sleepTest(JsonBox::Value& resultString, double delayTime, double sleepElapsed,
                double timeVariance, bool printFlag, bool assertFlag, int testno);

/**
 * Runs the tests for Timer
 *
 * @param tv1 The first timeval
 * @param tv2 The second timeval
 * @return True if the size tests runs properly
 */
bool objectIDSizeTest(bool printFlag, bool assertFlag, JsonBox::Value& resultString);

/**
 * Runs the tests for Timer
 *
 * @param tv1 The first timeval
 * @param tv2 The second timeval
 * @return True if thread pool things runs correctly
 */
bool doThreadPoolThing(int threads);
}