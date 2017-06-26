/******************************************************************************
 *
 * \file TSMap.cpp
 *
 *****************************************************************************/

#include "TSMap.tcc"

#include <atomic>
#include <chrono>
#include <future>
#include <iostream>
#include <limits>
#include <random>
#include <thread>
#include <string>
#include "assert.h"

using namespace atl;

// Test threads
bool find_random_from_map(TSMap<std::string, int>& map, int iter, int max, bool print)
{
    for (int i = 0; i < iter; i++) {
        int n = rand() % max;
        if (map.find("k"+std::to_string(n)).first != n) {
            if (print) {
                std::cout << "Error retrieving " << n << " from map" 
                          << std::endl;
            }
            return false;
        }
    }
    return true;
}

bool find_nonexistent(TSMap<std::string, int>& map, int iter, bool print)
{
    for (int i = 0; i < iter; i++) {
        int n = rand();
        if (map.find("nope_" + std::to_string(n)).second != false) {
            if (print) {
                std::cout << "Map returned true for finding nonexistent entry"
                          << std::endl;
            }
            return false;
        }
    }
    return true;
}

bool check_size_constant(TSMap<std::string, int>& map, int iter, bool print)
{
    size_t size = map.size();
    for (int i = 0; i < iter; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        size_t s = map.size();
        if (size != s) {
            if (print) {
                std::cout << "Map size changed from " << size
                          << " to " << s
                          << std::endl;
            }
            return false;
        }
    }
    return true;
}

bool check_size_increasing(TSMap<std::string, int>& map, int iter, int max, bool print)
{
    size_t size = map.size();
    for (int i = 0; i < iter; i++) {
        std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
        size_t s = map.size();
        if ((int)s == max) {
            break;
        } else if (s <= size) {
            if (print) {
                std::cout << "WARNING: Bad scheduling: writes pending but "
                          << "map size hasn't increased in 10 milliseconds: " 
                          << size << "-->" << s
                          << std::endl;
            }
            return false;
        }
        size = s;
    }
    return true;
}

bool add_to_map(TSMap<std::string, int>& map, 
                 std::vector<std::pair<std::string, int>> stuff, 
                 bool print)
{
    for (auto thing : stuff) {
        bool rc = map.emplace(thing.first, thing.second);
        if (!rc) {
            if (print) {
                std::cout << "Error adding ( " 
                          << thing.first << ", " << thing.second
                          << " ) to map"
                          << std::endl;
            }
            return false;
        }
    }
    return true;
}

JsonBox::Value testTSMap(bool printFlag, bool assertFlag, bool valgrind)
{
    JsonBox::Value resultString;
    std::cout << "Testing TSMap" << std::endl;

    //***********************************************
    // Test on a single thread for basic correctness
    //***********************************************
    if (printFlag) {
        std::cout << "Testing single-threaded operation..." << std::endl;
    }

    //test empty function
    TSMap<std::string, int> testMap;
    if (!testMap.empty()) {
        if (printFlag) {
            std::cout << "Empty map returned not empty" << std::endl;
        }
        if (assertFlag) {
            assert(false);
        }
        resultString["Empty map return"] = "fail";
        resultString["pass"] = false;
        return resultString;
    }
    resultString["Empty map"] = "pass";

    //check the size of an empty array
    size_t size = testMap.size();
    if (size != 0) {
        if (printFlag) {
            std::cout << "Empty map returned size " << size << std::endl;
        }
        if (assertFlag) {
            assert(false);
        }
        resultString["Empty map size return"] = "fail";
        resultString["pass"] = false;
        return resultString;
    }
    resultString["Empty map size return"] = "pass";

    //insert a few values
    for (int i = 0; i < 100; i++) {
        std::string s = "test" + std::to_string(i);
        if (!testMap.emplace(s, i)) {
            if (printFlag) {
                std::cout << "Error setting key-value pair ( " 
                      << s << ", " << i 
                      << " )" << std::endl;
            }
            if (assertFlag) {
                assert(false);
            }
            resultString["Insert"] = "fail";
            resultString["pass"] = false;
            return resultString;
        }
    }
    resultString["Insert"] = "pass";

    //check map size and test retrieving values
    size = testMap.size();
    if (size != 100) {
        if (printFlag) {
            std::cout << "Map returned size " << size
                  << " after inserting 100 elements into empty map"
                  << std::endl;
        }
        if (assertFlag) {
            assert(false);
        }
        resultString["Size"] = "fail";
        resultString["pass"] = false;
        return resultString;
    }
    resultString["Size"] = "pass";

    for (int i = 0; i < 100; i++) {
        std::string s = "test" + std::to_string(i);
        if (testMap.find(s).first != i) {
            if (printFlag) {
                std::cout << "Map returned " << i
                      << " for key " << s
                      << std::endl;
            }
            if (assertFlag) {
                assert(false);
            }
            resultString["Value return"] = "fail";
            resultString["pass"] = false;
            return resultString;
        }
    }
    resultString["Value return"] = "pass";

    //test deleting entries
    for (int i = 0; i < 50; i++) {
        std::string s = "test" + std::to_string(i);
        if (!testMap.erase(s)) {
            if (printFlag) {
                std::cout << "Error erasing key " << s << std::endl;
            }
            if (assertFlag) {
                assert(false);
            }
            resultString["Delete"] = "fail";
            resultString["pass"] = false;
            return resultString;
        }
    }
    resultString["Delete"] = "pass";

    if (testMap.size() != 50) {
        if (printFlag) {
            std::cout << "Map size returned " << testMap.size()
                  << " after erasing 50 of 100 entries"
                  << std::endl;
        }
        if (assertFlag) {
            assert(false);
        }
        resultString["Delete size return"] = "fail";
        resultString["pass"] = false;
        return resultString;
    }
    resultString["Delete size return"] = "pass";

    testMap.clear();
    if (!testMap.empty()) {
        if (printFlag) {
            std::cout << "Map not empty after clearing map" << std::endl;
        }
        if (assertFlag) {
            assert(false);
        }
        resultString["Clear map"] = "fail";
        resultString["pass"] = false;
        return resultString;
    }
    resultString["Clear map"] = "pass";

    if (printFlag) {
        std::cout << "Basic correctness testing (single thread) passed!" << std::endl;
    } 


    //***************************
    // Test multithreaded writes
    //***************************
    if (printFlag) {
        std::cout << "Testing multithreaded write access..." << std::endl;
        std::cout << "Generating test entries..." << std::endl;
    }

    const int numWrites = 5;
    const int numTests = valgrind ? 200 : 2000;
    std::vector<std::pair<std::string, int>> testEntries[numWrites];

    for (int i = 0; i < numTests; i++) {
        for (int j = 0; j < numWrites; j++) {
            testEntries[j].push_back(
                    std::make_pair(
                        "k"+std::to_string(numWrites*i+j), 
                        numWrites*i+j
                    ));
        }
    }

    if (printFlag) {
        std::cout << "...entries generated." << std::endl;
    }

    std::thread writeThreads[numWrites];
    bool writeFlags[numWrites];
    bool writeSuccess[numWrites];

    for (int i = 0; i < numWrites; i++) {
        writeFlags[i] = false;
        writeThreads[i] = std::thread(
                [&,i](){
                    writeSuccess[i] = add_to_map(std::ref(testMap),
                                                  testEntries[i],
                                                  printFlag);
                    std::cout << "writeThread " << i
                              << " exited with status " << writeSuccess[i]
                              << std::endl;
                    writeFlags[i] = true;
                });
    }

    while (1) {
        bool writeDone = true;
        for (int i = 0; i < numWrites; i++) {
            writeDone &= writeFlags[i];
        }
        if (writeDone) break;
    }

    //Test write threads
    for (int i = 0; i < numWrites; i++) {
        writeThreads[i].join();
        if (!writeSuccess[i]) {
            if (printFlag) {
                std::cout << "writeThread " << i << " failed!" << std::endl;
            }
            if (assertFlag) {
                assert(false);
            }
            resultString["Write thread"] = "fail";
            resultString["pass"] = false;
            return resultString;
        }
    }
    resultString["Write thread"] = "pass";

    if (printFlag) {
        std::cout << "Checking that all " << numTests*numWrites
                  << " entries were added..."
                  << std::endl;
    }

    //Test map size after writing threads
    if (testMap.size() != (unsigned)numTests * numWrites) {
        if (printFlag) {
            std::cout << "Write error: map size is " << testMap.size()
                  << " after trying to add " << numTests*numWrites
                  << " entries"
                  << std::endl;
        }
        if (assertFlag) {
            assert(false);
        }
        resultString["Write size"] = "fail";      
        resultString["pass"] = false;
        return resultString;
    }
    resultString["Write size"] = "pass";

    //If multithreaded writes pass...
    if (printFlag){
        std::cout << "Multithread write testing passed!" << std::endl;
    }


    //**************************
    // Test multithreaded reads
    //**************************
    if (printFlag) {
        std::cout << "Testing multithreaded read access..." << std::endl;
    }

    const int numReads = valgrind ? 5 : 10;
    std::thread readThreads[10];
    std::vector<bool> readFlags(numReads);
    std::vector<bool> readSuccess(numReads);

    for (int i = 0; i < numReads; i++) {
        readFlags[i] = false;
        if (i > (numReads/5 * 4)) {
            readThreads[i] = std::thread( 
                    [&,i](){ 
                        readSuccess[i] = find_nonexistent(std::ref(testMap), 
                                                          valgrind ? 200 : 20000, 
                                                             printFlag);
                        std::cout << "readThread " << i 
                                  << " (nonexistent) exited with status "
                                  << readSuccess[i]
                                  << std::endl;
                        readFlags[i] = true;
                    });
        } else if (i > 1) {
            readThreads[i] = std::thread( 
                    [&,i](){ 
                        readSuccess[i] = find_random_from_map(std::ref(testMap), 
                                                             valgrind ? 1000 : 10000, 
                                                             numTests*numWrites,
                                                             printFlag);
                        std::cout << "readThread " << i 
                                  << " exited with status " << readSuccess[i]
                                  << std::endl;
                        readFlags[i] = true;
                    });
        } else {
            readThreads[i] = std::thread( 
                    [&,i](){ 
                        readSuccess[i] = check_size_constant(std::ref(testMap), 
                                                             valgrind ? 25 : 1000, 
                                                             printFlag);
                        std::cout << "readThread " << i 
                                  << " (size checker) exited with status " 
                                  << readSuccess[i]
                                  << std::endl;
                        readFlags[i] = true;
                    });
        }
    }

    while (1) {
        bool readDone = true;
        for (int i = 0; i < numReads; i++) {
            readDone &= readFlags[i];
        }
        if(readDone) break;
    }

    for (int i = 0; i < numReads; i++) {
        readThreads[i].join();
        if (!readSuccess[i]) {
            if (printFlag) {
                std::cout << "readThread " << i << " failed!" << std::endl;
            }
            if (assertFlag) {
                assert(false);
            }
            resultString["Read thread"] = "fail";           
            resultString["pass"] = false;
            return resultString;
        }
    }
    resultString["Read thread"] = "pass";

    if (printFlag) {
        std::cout << "...test passed!" << std::endl;
    }


    //*******************************
    // Test simultaneous read-writes
    //*******************************
    if (printFlag) {
        std::cout << "Testing writing new entries while reading existing entries..."
                  << std::endl;
        std::cout << "Generating test entries..." << std::endl;
    }

    const int writes = valgrind ? 2 : 5;
    int tests = 2000;
    std::vector<std::pair<std::string, int>> testEntries2[5];

    for (int i = 0; i < tests; i++) {
        for (int j = 0; j < writes; j++) {
            testEntries2[j].push_back(
                    std::make_pair(
                        "s"+std::to_string(writes*i+j), 
                        writes*i+j
                    ));
        }
    }

    if (printFlag) {
        std::cout << "...entries generated." << std::endl;
    }

    const int reads = valgrind ? 3 : 8;
    const int counts = valgrind ? 1 : 2;
    std::thread sThreads[15];
    std::vector<bool> sFlags(writes + reads + counts);
    std::vector<bool> sSuccess(writes + reads + counts);

    for (int i = 0; i < writes+reads+counts; i++) {
        sFlags[i] = false;
        if (i < writes) {
            sThreads[i] = std::thread(
                    [&,i](){
                        sSuccess[i] = add_to_map(std::ref(testMap),
                                                      testEntries2[i],
                                                      printFlag);
                        std::cout << "writeThread " << i
                                  << " exited with status " << sSuccess[i]
                                  << std::endl;
                        sFlags[i] = true;
                    });
        } else if (i < writes+reads) {
            sThreads[i] = std::thread( 
                    [&,i](){ 
                        sSuccess[i] = find_random_from_map(std::ref(testMap), 
                                                             valgrind ? 4000 : 10000, 
                                                             numTests*numWrites,
                                                             printFlag);
                        std::cout << "readThread " << i 
                                  << " exited with status " << sSuccess[i]
                                  << std::endl;
                        sFlags[i] = true;
                    });
        } else {
            sThreads[i] = std::thread(
                    [&,i](){
                        sSuccess[i] = check_size_increasing(std::ref(testMap),
                                                            valgrind ? 400 : 1000,
                                                            numTests*numWrites + tests*writes,
                                                            printFlag);
                        std::cout << "readThread " << i
                                  << " (size checker) exited with status "
                                  << sSuccess[i]
                                  << std::endl;
                        sFlags[i] = true;
                    });
        }
    }

    while (1) {
        bool sDone = true;
        for(int i = 0; i < writes+reads+counts; i++){
            sDone &= sFlags[i];
        }
        if(sDone) break;
    }

    if (printFlag) {
        std::cout << "Checking that all entries were added..." << std::endl;
    }

    if ((int)testMap.size() != numTests*numWrites + tests*writes) {
        if (printFlag) {
            std::cout << "Write error: not all entries added to map" << std::endl;
        }
        if (assertFlag) {
            assert(false);
        }
        resultString["Write entries to map"] = "fail";
        resultString["pass"] = false;
        return resultString;
    }
    resultString["Write entries to map"] = "pass";

    for (int i = 0; i < writes+reads+counts; i++) {
        sThreads[i].join();
        if (!sSuccess[i]) {
            if (printFlag) {
                std::cout << "Thread " << i << " failed!" << std::endl;
                if (i < writes+reads) {
                    resultString["Thread join"] = "fail";
                    resultString["pass"] = false;
                    return resultString;
                }
            }
            if (assertFlag) {
                assert(false);
            }
            resultString["pass"] = false;
            return resultString;
        }
    }
    resultString["Thread join"] = "pass";

    if (printFlag) {
        std::cout << "Simultaneous read-write testing passed!" << std::endl;
    }


    //********************************************
    // Test that reads are not mutually exclusive
    //********************************************
    if (!valgrind) {
        if (printFlag) {
            std::cout << "Testing that reads are not mutually exclusive..."
                      << std::endl;
        }

        std::atomic<int> checker(0);
        int numReadIterations = 500000;
        const int sReads = 4;
        std::thread sReadThreads[sReads];
        bool sReadFlags[sReads];
        bool sReadSuccess[sReads];

        for (int i = 0; i < sReads; i++) {
            sReadFlags[i] = false;
            sReadThreads[i] = std::thread( 
                    [&,i](){ 
                        int notSimul = 0;
                        for( int j = 0; j < numReadIterations; j++ ){
                            int n = rand() % 10000;
                            int before = checker.load();
                            testMap.find( "k"+std::to_string(n) );
                            checker++;
                            if( checker.load() <= before+1 ){
                                notSimul++;
                            }
                        }
                        sReadSuccess[i] = (notSimul < numReadIterations*0.4);
                        sReadFlags[i] = true;
                        std::cout << notSimul 
                                  << " out of "
                                  << numReadIterations
                                  << " iterations happened alone"
                                  << std::endl;
                    });
        }
        while (1) {
            bool sReadDone = true;
            for (int i = 0; i < sReads; i++) {
                sReadDone &= sReadFlags[i];
            }
            if (sReadDone) break;
        }

        int c = 0;
        for (int i = 0; i < sReads; i++) {
            sReadThreads[i].join();
            if (!sReadSuccess[i]) {
                c++;
            }
        }

        if (c >= sReads/2) {
            if (printFlag) {
                std::cout << "Multiple threads failed to read concurrently"
                          << std::endl;
            }
            if (assertFlag) {
                assert(false);
            }
            resultString["Mult. thread read"] = "fail";
            resultString["pass"] = false;
            return resultString;
        }
        resultString["Mult. thread read"] = "pass";

        if (printFlag) {
            std::cout << "Mutual exclusivity read test passed!" << std::endl;
        }
    }


    //***************************
    // Test conditional deletion
    //***************************
    if (printFlag) {
        std::cout << "Testing conditional deletion..." << std::endl;
        std::cout << "Generating test entries..." << std::endl;
    }

    TSMap<int, std::string> testMap2;
    std::string truePrefix = "delete_this_";
    int numTrueEntries = valgrind ? 300 : 3000;
    int numFalseEntries = valgrind ? 700 : 7000;

    for (int i = 0; i < numTrueEntries; i++) {
        testMap2.emplace(i, truePrefix + std::to_string(i));
    }

    for (int i = 0; i < numFalseEntries; i++) {
        testMap2.emplace(i+numTrueEntries, "keep_this_" + std::to_string(i));
    }

    if ((int)testMap2.size() != numTrueEntries+numFalseEntries) {
        if (printFlag) {
            std::cout << "Error adding entries to map. Only "
                  << testMap2.size()
                  << " entries of "
                  << numTrueEntries + numFalseEntries
                  << " added"
                  << std::endl; 
        }
        if (assertFlag) {
            assert(true);
        }
        resultString["Entry add"] = "fail";  
        resultString["pass"] = false;
        return resultString;
    }
    resultString["Entry add"] = "pass";

    if (printFlag) {
        std::cout << "...entries generated." << std::endl;
        std::cout << "Attempting to delete " << numTrueEntries
                  << " out of " << numTrueEntries+numFalseEntries
                  << " elements..."
                  << std::endl;
    }

    int numDeleted = testMap2.delete_if(
            [&](int k, std::string& v) -> bool{
                if (v.find(truePrefix) != std::string::npos) {
                    if (printFlag) {
                        std::cout << "Delete attempt failed" << std::endl;
                    }
                    return false;
                }
                return true;
            }
    );

    if((numDeleted != numTrueEntries) 
        || ((int)testMap2.size() != numFalseEntries)) {
        if (printFlag) {
            std::cout << "Conditional delete function deleted "
                  << numDeleted << " of " << numTrueEntries
                  << " elements to delete. Map size after deletion is "
                  << testMap2.size() << " instead of " << numFalseEntries
                  << std::endl;
        }
        if (assertFlag) {
            assert(false);
        }
        resultString["Conditional delete"] = "fail";         
        resultString["pass"] = false;
        return resultString;
    }
    resultString["Conditional delete"] = "pass";

    if (printFlag) {
        std::cout << "Successfully deleted " << numDeleted << " elements"
                  << std::endl;
        std::cout << "Map size after conditional deletion: "
                  << testMap2.size()
                  << std::endl;
        std::cout << "Conditional deletion test passed!" << std::endl;
    }


    //********************************
    // Test for-each function passing
    //********************************
    if (printFlag) {
        std::cout << "Testing for-each read-only function passing..." 
                  << std::endl;
        std::cout << "Generating test entries..." << std::endl;
    }

    TSMap<int, int> testMap3;
    int numFEntries = valgrind ? 10000 : 100000;
    const int numCounters = 4;
    std::thread countThreads[numCounters];
    bool countFlags[numCounters];
    size_t countReturns[numCounters];
    int counters[numCounters];

    for (int i = 0; i < numFEntries; i++) {
        testMap3.emplace(i, i);
    }

    if (printFlag) {
        std::cout << "...entries generated." << std::endl;
    }

    for (int i = 0; i < numCounters; i++) {
        countFlags[i] = false;
        counters[i] = 0;
        countThreads[i] = std::thread( [&,i](){
                countReturns[i] = testMap3.for_each_ro( 
                        [&,i](int key, int value)->bool {
                            //std::cout << "counter " << i << std::endl;
                            if(!(value % (i+1))){
                                //std::cout << "incrementing " << i+1 << std::endl;
                                counters[i]++;
                            }
                            return true;
                        }
                );
                countFlags[i] = true;
        });
    }

    while (1) {
        bool countDone = true;
        for (int i = 0; i < numCounters; i++) {
            countDone &= countFlags[i];
        }
        if (countDone) break;
    }

    for (int i = 0; i < numCounters; i++) {
        countThreads[i].join();

        if ((int)countReturns[i] != numFEntries) {
            if (printFlag) {    
                std::cout << "Thread " << i << " only acted on "
                      << countReturns[i] << " elements"
                      << std::endl;
            }
            if (assertFlag) {
                assert(false);
            }
            resultString["Thread act"] = "fail";  
            resultString["pass"] = false;
            return resultString;       
        }

        if (counters[i] != (int)ceil((float)numFEntries/(i+1))) {
            if (printFlag) {
                std::cout << "Thread " << i 
                      << " returned wrong count of integers mod " << i+1
                      << ": " << counters[i]
                      << std::endl;
            }
            if (assertFlag) {
                assert(false);
            }
            resultString["Thread count"] = "fail";
            resultString["pass"] = false;
            return resultString;
        }
    }
    resultString["Thread act"] = "pass";
    resultString["Thread count"] = "pass";

    if (printFlag) {
        std::cout << "For-each read-only testing passed!" << std::endl;
    }

    if (printFlag) {
        std::cout << "Testing for-each function passing..." 
                  << std::endl;
        std::cout << "Generating test entries..." << std::endl;
    }

    size_t r = testMap3.for_each( 
            [](int key, int& value)->bool {
                value = value % 2;
                return true;
            }
    );

    if (r != testMap3.size()) {
        if (printFlag) {
            std::cout << "Function passing acted on " << r << " of " 
                  << testMap.size() << " elements"
                  << std::endl;
        }
        if (assertFlag) {
            assert(false);
        }
        resultString["Function act"] = "fail";
        resultString["pass"] = false;
        return resultString;
    }
    resultString["Function act"] = "pass";

    size_t numZeros = testMap3.for_each_ro( 
            [](int k, int v)->bool{
                if (v == 0) return true;
                return false;
            }
    );

    size_t numOnes = testMap3.for_each_ro( 
            [](int k, int v)->bool{
                if (v == 1) return true;
                return false;
            }
    );

    if (((int)numZeros != numFEntries/2) || ((int)numOnes != numFEntries/2)) {
        if (printFlag) {
            std::cout << "Function passing changed " << numZeros
                  << " values to 0 and " << numOnes
                  << " values to 1. Both should be " << numFEntries/2
                  << std::endl;            
        }
        if (assertFlag) {
            assert(false);
        }
        resultString["Function change"] = "fail";
        resultString["pass"] = false;
        return resultString;
    }
    resultString["Function change"] = "pass";

    if (printFlag) {
        std::cout << "For-each function testing passed!" << std::endl;
    }


    //*************************************
    // Pound the map with a ton of threads
    // ************************************
//for( int z = 0; z < 1000; z++ ){
//    std::cout << "\nThreadbomb " << z << std::endl;
    int numThreads;
    if (valgrind) {
        numThreads = 50;
    } else{
        numThreads = 10000;
    }

    if (printFlag) {
        std::cout << "Testing with " << numThreads << " threads..."
                  << std::endl;
    }

    TSMap<int, int> map;
    for (int i = numThreads/2; i < numThreads*2; i++) {
        map.emplace(i, i);
    }

    std::promise<void> go;
    std::shared_future<void> ready_future(go.get_future());
    std::thread* threads = new std::thread[numThreads];

    for (int i = 0; i < numThreads; i++) {
        if (i % 9 == 0) { //write 100 random values
            threads[i] = std::thread(
                    [&](){
                        ready_future.wait();
                        for (int j = 0; j < 100; j++) {
                            int t = rand() % numThreads;
                            map.emplace(t, t);
                        }
                        //std::cout << "emplace thread done" << std::endl;
                    });
        } else if (i % 9 == 1) { //find 100 random keys
            threads[i] = std::thread(
                    [&](){
                        ready_future.wait();
                        for (int j = 0; j < 100; j++) {
                            int t = rand() % (numThreads*2);
                            map.find(t);
                        }
                        //std::cout << "find thread done" << std::endl;
                    });
        } else if (i % 9 == 2) { //lower_bound 100 random keys
            threads[i] = std::thread(
                    [&](){
                        ready_future.wait();
                        for (int j = 0; j < 100; j++) {
                            int t = rand() % (numThreads*2);
                            map.lower_bound(t);
                        }
                        //std::cout << "lower_bound thread done" << std::endl;
                    });
        } else if (i % 9 == 3) { //erase 100 random keys
            threads[i] = std::thread(
                    [&](){
                        ready_future.wait();
                        for (int j = 0; j < 100; j++) {
                            int t = rand() % numThreads;
                            map.erase(t);
                        }
                        //std::cout << "erase thread done" << std::endl;
                    });
        } else if (i % 9 == 4) { //query the map size 100 times
            threads[i] = std::thread(
                    [&](){
                        ready_future.wait();
                        for(int j = 0; j < 100; j++) {
                            map.size();
                        }
                        //std::cout << "size thread done" << std::endl;
                    });
        } else if (i % 9 == 5) { //check if the map is empty 100 times
            threads[i] = std::thread(
                    [&](){
                        ready_future.wait();
                        for (int j = 0; j < 100; j++) {
                            map.empty();
                        }
                        //std::cout << "empty thread done" << std::endl;
                    });
        } else if (i % 9 == 6) { //spawn a read-only function iterator
            threads[i] = std::thread(
                    [&](){
                        ready_future.wait();
                        map.for_each_ro( 
                                [](const int k, const int v)->bool{
                                    if (k == v) {
                                        return true;
                                    }
                                    return false;
                                });
                        //std::cout << "for_each_ro thread done" << std::endl;
                    });
        } else if (i % 9 == 7) { //spawn a function iterator
            threads[i] = std::thread(
                    [&](){
                        ready_future.wait();
                        map.for_each_ro( 
                                [](int k, int v)->bool{
                                    v += rand() % 1000;
                                    return true;
                                });
                        //std::cout << "for_each thread done" << std::endl;
                    });
        } else if (i % 9 == 8) { //spawn a conditional deleter
            threads[i] = std::thread(
                    [&](){
                        ready_future.wait();
                        map.delete_if( 
                                [](int k, int v)->bool{
                                    if (k != v) {
                                        return true;
                                    } 
                                    return false;
                                });
                        //std::cout << "delete_if thread done" << std::endl;
                    });
        }
    }

    go.set_value();

    for (int i = 0; i < numThreads; i++) {
        threads[i].join();
    }

    if (printFlag) {
        std::cout << "Misc. testing passed!" << std::endl;
    }

    delete [] threads;

    //Test getKeyList()
    if (printFlag) {
        std::cout << "Testing getKeyList()..." << std::endl;
    }

    TSMap<std::string, std::string> testMap4;
    std::vector<std::string> keyListIn;

    for (int i = 0; i < 5; i++) {
        std::string k = "key";
        keyListIn.push_back(k + std::to_string(i));
        testMap4.emplace(k + std::to_string(i), std::to_string(i));
    }

    std::vector<std::string> keyListOut = testMap4.getKeyList();
    for (int i = 0; i < 5; i++) {
        if (keyListIn[i] != keyListOut[i]) {
            if (printFlag) {
                std::cout << "Error: keyListIn, keyListOut: "
                      << keyListIn[i] << ", " << keyListOut[i]
                      << std::endl;
            }
            if (assertFlag) {
                assert(false);
            }
            resultString["getKeyList()"] = "fail";
            resultString["pass"] = false;
            return resultString;
        } else {
            resultString["getKeyList()"] = "pass";
        }
    }

    std::cout << "TSMap Unit Test passed!\n" << std::endl;
    resultString["pass"] = true;
    return resultString;
}
