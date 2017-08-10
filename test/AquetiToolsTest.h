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
#include <TSMap.tcc>
#include <revision.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <TSQueue.tcc>
#include "ThreadPool.h"
#include "TaskManager.tcc"
#include <assert.h>
#include "mongoapi/mongoapi.h"
#include <ctime>

namespace atl{

/**
 * Perform unit tests for the aquetitools repository
 *
 * @param unitList The list of units to run tests on.
 * @param testSubmodules The boolean, if true run unit tests on submodules as well.
 * @param printFlag A boolean, if true tests print out messages to the console
 * @param assertFlag A boolean, if true program halts on error
 * @param valgrind The boolean, if true run with valgrind settings.
 * @return JsonBox value of the test results
 */
JsonBox::Value testAquetiTools(bool testSubmodules = true, bool printFlag = true, bool assertFlag = false, bool valgrind = false, 
	std::vector<std::string> unitList = {"Timer", "CRC", "Thread", "MultiThread", "ThreadPool", 
	"LruCache", "TSMap", "TSQueue", "TaskManager"});

/**
 * Runs the tests for Timer
 *
 * @param printFlag A boolean, if true tests print out messages to the console
 * @param assertFlag A boolean, if true program halts on error
 * @return JsonBox value of the test results
 */
JsonBox::Value testTimer(bool printFlag = true, bool assertFlag = false);

/**
 * Runs the tests for Thread
 *
 * @param printFlag A boolean, if true tests print out messages to the console
 * @return JsonBox value of the test results
 */
JsonBox::Value testThread(bool printFlag = true);

/**
 * Runs the tests for MultiThread
 *
 * @param printFlag A boolean, if true tests print out messages to the console
 * @return JsonBox value of the test results
 */
JsonBox::Value testMultiThread(bool printFlag = true);

/**
 * Runs the tests for ThreadPool
 *
 * @return JsonBox value of the test results
 */
JsonBox::Value testThreadPool();

/**
 * Runs the tests for LruCache
 *
 * @param numThreads The number of threads used during unit testing (default 100)
 * @param printFlag A boolean, if true tests print out messages to the console
 * @param assertFlag A boolean, if true program halts on error
 * @return JsonBox value of the test results
 */
JsonBox::Value testLruCache(unsigned int numThreads = 100, bool printFlag = true, bool assertFlag = false);

/**
 * Runs the tests for TSQueue
 *
 * @param numThreads The number of threads used during unit testing (default 20)
 * @param printFlag A boolean, if true tests print out messages to the console
 * @param assertFlag A boolean, if true program halts on error
 * @return JsonBox value of the test results
 */
JsonBox::Value testTSQueue(unsigned int numThreads = 20, bool printFlag = true, bool assertFlag = false);

/**
 * Runs the tests for TSMap
 *
 * @param printFlag A boolean, if true tests print out messages to the console
 * @param assertFlag A boolean, if true program halts on error
 * @param valgrind A boolean, if true sets valgrind settings for unit testing
 * @return JsonBox value of the test results
 */
JsonBox::Value testTSMap(bool printFlag = false, bool assertFlag = false, bool valgrind = false);

/**
 * Runs the tests for TaskManager
 *
 * @param threads The number of threads used for testing (default 1000)
 * @param printFlag A boolean, if true tests print out messages to the console
 * @param assertFlag A boolean, if true program halts on error
 * @param valgrind A boolean, if true sets valgrind settings for unit testing
 * @return JsonBox value of the test results
 */
JsonBox::Value testTaskManager(int threads = 1000, bool printFlag = true, bool assertFlag = false, bool valgrind = false);

}
