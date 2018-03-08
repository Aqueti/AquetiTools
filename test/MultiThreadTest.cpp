/**
 * \file MultiThreadTest.cpp
 **/

#include "AquetiToolsTest.h"

namespace atl {
/**
* \brief Test function
* 
* \param [in] printFlag boolean (true prints info to console)
* \return true if the test succeeded
**/
JsonBox::Value testMultiThread(bool printFlag)
{
    JsonBox::Value resultString; //!< Brief JsonBox value with unit test results
 
    bool rc = true;
    MultiThread mThread(5);

    if (printFlag) {
        std::cout << "Test Stop function" << std::endl;
    }

    mThread.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    mThread.Stop();
    rc = mThread.Join();

    mThread.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    mThread.Stop();
    rc = mThread.Join() && rc;

    if (!rc) {
        if (printFlag) {
            std::cout << "Unable to join thread!" << std::endl;
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

    if (resultString["pass"] == false) {
        return resultString;
    }
    resultString["pass"] = true;
    return resultString;
}
}
