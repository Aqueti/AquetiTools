/**
 * \file ThreadTest.cpp
 **/

#include "AquetiToolsTest.h"

namespace atl {
/**
 * \brief Test function
 **/
JsonBox::Value testThread(bool printFlag)
{
    JsonBox::Value resultString; //!< Brief JsonBox value with unit test results

    bool rc = true;
    Thread cThread;

    if (printFlag) {
        std::cout << "Test Stop function" << std::endl;
    }

    cThread.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    cThread.Stop();
    cThread.Join();

    cThread.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    //Test join
    cThread.Stop();
    rc = cThread.Join();

    if (!rc) {
        if (printFlag) {
            std::cout << "Unable to join thread!"<<std::endl;
            std::cout << "Thread test error. See ThreadTest.log" << std::endl;
        }
        resultString["Join"] = "fail";
        resultString["pass"] = false;
    } else {
        resultString["Join"] = "pass";
    }

    if (printFlag) {
        std::cout << "Joined" << std::endl;
    }

    //Vector tests (timed)
    size_t threadCount = 50;
    std::vector<Thread> threadVect(threadCount);

    if (printFlag) {
        std::cout << "Testing vector "<<std::endl << std::endl;
    }

    //Spawn threadCount threads
    for (uint16_t i = 0; i < threadCount; i++) {
        threadVect[i].Start();
    }


    //Join all
    for (uint16_t i = 0; i < threadCount; i++) {
        threadVect[i].Stop();
        threadVect[i].Join();
    }

    //ReSpawn threadCount threads
    for (uint16_t i = 0; i < threadCount; i++) {
        threadVect[i].Start();
    }

    //Stop threadCount threads
    for (unsigned int i = 0; i < threadCount; i++) {
        threadVect[i].Stop();
        threadVect[i].Join();
    }

    if (resultString["pass"] == false) {
        return resultString;
    }
    resultString["pass"] = true;
    return resultString;
}
}
