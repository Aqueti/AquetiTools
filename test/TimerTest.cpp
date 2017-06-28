#include "AquetiToolsTest.h"

namespace atl{
//***********************************************************
/*!\brief functional test for this class
 *
 * \param [in] printFlag boolean (true prints info to console)
 * \param [in] assertFlag boolean (true stops the program)
 * \return JsonBox::Value containing test results
 */
//***********************************************************
JsonBox::Value testTimer(bool printFlag, bool assertFlag)
{
    JsonBox::Value resultString;
    Timer timer;

    double timeVariance = (double) 2.0 / 1e3;
    double currTime = getTime();

    if (printFlag) {
        std::cout << "Regression test for Timer class" << std::endl;
    }

    timer.setFPS(30.0);

    //Check ObjectId size
    if (!objectIDSizeTest(printFlag, assertFlag, resultString)) {
        return resultString;
    }

    //Test sleep time
    timer.start();
    double delayTime = 10.0 / 1e3;
    sleep(delayTime);
    double sleepElapsed = timer.elapsed();

    if (!sleepTest(resultString, delayTime, sleepElapsed, timeVariance, 
    			printFlag, assertFlag, 1)) {
        return resultString;
    }

    //Test a longer sleep time
    timer.start();
   	delayTime = 1.0;
   	sleep(delayTime);
   	sleepElapsed = timer.elapsed();

	if (!sleepTest(resultString, delayTime, sleepElapsed, timeVariance, 
				printFlag, assertFlag, 2)) {
        return resultString;
    }
 
   	//Get current timestamp
    int64_t tstamp = convertDoubleToTimeStamp(currTime);

    if (printFlag) {
        std::cout << "Current timestamp: " << tstamp << std::endl;
    }

    resultString["timestamp"] = (double) tstamp;

    //Get elapsed timestamp
    timer.start();
    double tElapsed = timer.elapsed();
    int64_t tstampElapsed = convertDoubleToTimeStamp(tElapsed);

    if (printFlag) {
    	std::cout << "Timer since second start (can be 0 if <1msec): " 
    				<< tstampElapsed << std::endl;
    }

    resultString["updated timestamp"] = (double) tstampElapsed;

    //Get SMPTE time
    SMPTETime currsmpte = convertDoubleToSMPTE(currTime, timer.getFPS());

    if (printFlag) {
    	printf("SMPTETime: %2d:%2d:%2d, frame: %d\n"
                , currsmpte.hour
                , currsmpte.minute
                , currsmpte.second
                , currsmpte.frame
              );
    }

    resultString["smpte hour"] = currsmpte.hour;
    resultString["smpte minute"] = currsmpte.minute;
    resultString["smpte second"] = currsmpte.second;
    resultString["smpte frame"] = currsmpte.frame;

    //Get string time
    timeval tempStr = convertDoubleToTimeVal(currTime);
    std::string timeStr = convertTimeValToString(tempStr, timer.getFPS());

    if (printFlag) {
    	std::cout << "String time: " << timeStr.c_str() << std::endl;
    }

    resultString["string time"] = timeStr;

    //Get time code
    timeval tempCode = convertDoubleToTimeVal(currTime);
    int64_t timeCode = convertTimeValToTimeCode(tempCode, timer.getFPS());

    if (printFlag) {
    	std::cout << "Time code: " << timeCode << std::endl;
    }

    resultString["time code"] = (double) timeCode;

    /*ObjectId t1;
    ObjectId t2;
    t1.m_value = getTimestamp();
    sleep(1);
    t2.m_value = getTimestamp();

    ObjectId ref;
    ref.m_id  = 0;
    ref.m_utc = 123132;

    for( int i = 0; i < 65535; i++ ) {
        ref.m_step = i;

        double r1 = convertObjectIdTimeToDouble(ref);
        ObjectId ref2 = convertDoubleToObjectIdTime( r1 );

	resultString["conversionTime"]["time1"].setDouble(ref.m_step);
	resultString["conversionTime"]["time2"].setDouble(ref2.m_step);

        if((ref.m_utc != ref2.m_utc)||
                (ref.m_step - ref2.m_step > 1)||
                (ref2.m_step - ref.m_step > 1)) {
            if(printFlag) {
                timerTest << "Time difference in conversion\n"
                          << "   "<<ref.m_step << "!= "<<ref2.m_step <<std::endl;

		resultString["conversionTime"]["result"].setString("fail");

                cout << "Timer test failed. See TimerTest.log" << endl;


            }
            if (assertFlag) {
                assert(false);
            };
            return false;
        }
	resultString["conversionTime"]["result"].setString("pass");
   }

   
    double time1 = convertObjectIdTimeToDouble(t1);
//      double time1 = (double)t1.m_utc + STEP_SIZE * (double)t1.m_step/(double)TIMER_STEP;
    double time2 = (double)t2.m_utc + STEP_SIZE * (double)t2.m_step/(double)TIMER_STEP;

    resultString["conversionDelay"]["value"].setDouble(time2-time1);
    resultString["conversionDelay"]["allowedDelay"].setDouble(allowedDelay);

    if( time2 - time1 >  1 + allowedDelay ) {
        if(printFlag) {
            timerTest << "Time difference is large. Expected 1.0 +/- "<<allowedDelay<<"\n"
                      << " Actual: "<<time2<<"-"<<time1<<" = "<<time2 -time1 <<std::endl;

	    resultString["conversionDelay"]["result"].setString("fail");

            cout << "Timer test failed. See TimerTest.log" << endl;
        }
        if (assertFlag) {
            assert(false);
        };
        return false;
    }
    resultString["conversionDelay"]["result"].setString("pass");

   
    try {
      std::cout << "Should be date: " << getDateAsString() << std::endl;
    } catch (...) {
      cout << "getDateAsString() threw exception" << std::endl;
      return false;
    }

    timerJson << resultString << std::endl;

    //std::system("rm TimerTest.log"); */

    resultString["pass"] = true;
    return resultString;
}

/**
 * \brief Helper function to check the ObjectID size
 *
 * \param [in] printFlag boolean (true prints info to console)
 * \param [in] assertFlag boolean (true stops the program)
 * \param [in] resultString JsonBox value passed by reference, updated only if this test fails
 **/
bool objectIDSizeTest(bool printFlag, bool assertFlag, JsonBox::Value& resultString) 
{
    if (sizeof(uint64_t) != sizeof(ObjectId)) {
        if (printFlag) {
            std::cout << "size uint64_t: " << sizeof(uint64_t) << "!= ObjectId: " << sizeof(ObjectId) << "!\n"
                      << "Check variable alignment in ObjectId structure." << std::endl;

            std::cout << "Timer test failed. See TimerTest.log" << std::endl;
        }

        if (assertFlag) {
            assert(false);
        };

        resultString["pass"] = false;
        return false;
    }
    return true;
}

/**
 * \brief Helper function to test sleep
 *
 * \param [in] resultString JsonBox value passed by reference, updated with results of this test
 * \param [in] delayTime double delay expected of sleep function
 * \param [in] sleepElapsed double actual elapsed time of sleep function
 * \param [in] timeVariance double acceptable variance between delayTime and sleepElapsed
 * \param [in] printFlag boolean (true prints info to console)
 * \param [in] assertFlag boolean (true stops the program)
 * \param [in] testno int that keeps track of current test number (either 1 or 2)
 *
 *  Calculates the variance between the delay time passed in and the actual sleep time
 *  elapsed, and checks this against an allowed time variance. Notifies user that the
 *  test has been passed if actual variance is less than the time variance.
 **/
bool sleepTest(JsonBox::Value& resultString, double delayTime, double sleepElapsed,
				double timeVariance, bool printFlag, bool assertFlag, int testno) 
{
	std::string currTest = "sleep time " + std::to_string(testno);

	resultString[currTest] = sleepElapsed;
    double variance = delayTime - sleepElapsed;

    if (variance < 0) {
    	variance = -variance;
    }

    resultString[currTest]["variance"].setDouble(variance);
    resultString[currTest]["delay time"].setDouble(delayTime);
    resultString[currTest]["elapsed time"].setDouble(sleepElapsed);

    if (variance > timeVariance) {
    	if (printFlag) {
    		std::cout << currTest << ": " << std::fixed << delayTime << "failed: " 
    					<< sleepElapsed << ">" << delayTime + timeVariance << std::endl;
    		std::cout << "Timer test failed. See TimerTest.log" << std::endl;
    	}

    	if (assertFlag) {
    		assert(false);
    	};

    	resultString[currTest]["status"] = "fail";
    	resultString["pass"] = false;
    	return false;
    }

    return true;
}
}