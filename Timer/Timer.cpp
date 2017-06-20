//   Copyright Aqueti 2016.

// Distributed under the Boost Software License, Version 1.0.

//    (See accompanying file LICENSE_1_0.txt or copy at

//   http://www.boost.org/LICENSE_1_0.txt)

//***********************************************************
/*!\file Timer.cpp
 * \brief Timer class provides timing functionality for testing
 * program performance
 *
 * \author S. D. Feller 2014
 *************************************************************/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <thread>
#include <chrono>
#include <limits>
#include <iomanip>
#include <assert.h>
#include <fstream>
#include <iomanip>
#include <string>
#include <sstream>
#include <JsonBox.h>

#ifdef UNIX
#else
#include <winsock2.h>
#endif
//#include <curl/curl.h>
//#include <JsonBox.h>
#include "Timer.h"
#include <time.h>

using namespace std;

ofstream timerTest;
ofstream timerJson;

const double STARTING_FPS = 30.0;

namespace atl
{
//************************************************************
/*!\brief Timer constructor
 */
//************************************************************
Timer::Timer()
{

    //Initialize random number generator
//      srand( time(NULL));
    start();
}

//***********************************************************
/*!\brief Timer destructor
 */
//************************************************************/
Timer::~Timer()
{
}

//***********************************************************
/*!\brief Sets the number of fps in timer
 */
//************************************************************
void Timer::setFPS(double rate)
{
    m_fps = rate;
}

//***********************************************************
/*!\brief returns the number of fps in timer
 */
//************************************************************
double Timer::getFPS()
{
    return m_fps;
}


//***********************************************************
/**
 *!\brief returns the current timecode
 *
 * This is an internal function since it will eventually calculate
 * frame rate and other information
 **/
//************************************************************
SMPTETime Timer::getTimeCode()
{
    double now = getTime() + m_timeCodeOffset;

    if(m_fps == 0) {
        timerTest <<"Timer::getTimeCode unknown fps."<<std::endl;
    }

    return(convertDoubleToSMPTE(now, m_fps));
}


//***********************************************************
/*!\brief returns the timeCode offset to match system time with global time
 */
//************************************************************
void Timer::updateTimeCodeOffset(int64_t refTime)
{
    /*
    //Get current time
    timeval tv;
    gettimeofday(&tv, NULL);

    timeCodeOffset = refTime - convertTimeValToTimeCode( tv, fps );
    */

    return;
}


//***********************************************************
/*!\brief returns the timeCode offset to match system time with global time
 */
//************************************************************
int64_t Timer::getTimeCodeOffset()
{
    return m_timeCodeOffset;
}


//***********************************************************
/*!\brief starts timer
 *
 * Resets the start time and becomes the new point of reference
 */
//************************************************************/
void Timer::start()
{
    m_startTime = getTime();
    return;
}

//***********************************************************
/*!\brief starts timer
 *
 * Resets the start time and becomes the new point of reference
 */
//************************************************************/
double Timer::elapsed()
{
    double m_stopTime = getTime();
    return  m_stopTime - m_startTime;
}

//***********************************************************
/*!\brief converts a timeval struct to double
 */
//************************************************************/
double convertTimeValToDouble(timeval tv)
{
    return (double)tv.tv_sec + (double)((int)tv.tv_usec) / double(TIMER_STEP);
}

//***********************************************************
/*!\brief converts a double to a timeval struct
 */
//************************************************************/
timeval convertDoubleToTimeVal(double dTime)
{
    timeval tv;
    tv.tv_sec = dTime;
    tv.tv_usec = (dTime-tv.tv_sec) * 1e6;

    return tv;
}

//***********************************************************
/**
 *!\brief converts a double to an SMPTE time.
 *
 * \param [in] dTime time to convert
 * \param [in] fps number of frames per second (used to convert decimal time to a frame)
 *
 **/
//***********************************************************
SMPTETime convertDoubleToSMPTE(double dTime, double fps)
{
    timeval tv = convertDoubleToTimeVal(dTime);
    SMPTETime smpte = convertTimeValToSMPTE(tv, fps);

    return smpte;
}

//***********************************************************
/*!\brief converts a SMPTE to Timecode (hhmmssff)
 */
//************************************************************/
int64_t convertSMPTEToTimeCode(SMPTETime smpte)
{
    return smpte.hour*1e6 + smpte.minute*1e4+smpte.second*1e2+smpte.frame;
}

//***********************************************************
/*!\brief converts a timeval structure to Timecode (hhmmssff)
 */
//************************************************************/
int64_t convertTimeValToTimeCode(timeval tv, double fps)
{
    SMPTETime smpte = convertTimeValToSMPTE(tv, fps);

    return convertSMPTEToTimeCode(smpte);
}

//***********************************************************
/**
 *!\brief converts a double to an SMPTE time.
 *
 * \param [in] tv time  to convert
 * \param [in] fps number of frames per second (used to convert decimal time to a frame)
 *
 **/
//***********************************************************
SMPTETime convertTimeValToSMPTE(timeval tv, double fps)
{
    SMPTETime smpte;
    memset(&smpte, 0, sizeof(SMPTETime));
    //Convert timestamp to current time
    struct tm* tmptr = localtime((time_t*)&tv.tv_sec);

    if (tmptr != nullptr) {
      smpte.hour = tmptr->tm_hour;
      smpte.minute = tmptr->tm_min;
      smpte.second = tmptr->tm_sec;
      smpte.frame = (int)fps * (double(tv.tv_usec) / double(TIMER_STEP));
    }

    return smpte;
}

//***********************************************************
/**
 * \brief Converts the given timeval and int into a a string
 **/
//***********************************************************
std::string convertTimeValToString(timeval tv, double fps)
{
    char buffer[256];

    SMPTETime smpte = convertTimeValToSMPTE(tv, fps);

    sprintf( buffer, "%02d:%02d:%02d:%02d"
             , smpte.hour
             , smpte.minute
             , smpte.second
             , smpte.frame
           );

    std::string result;
    result.assign(buffer);

    return result;
}

//***********************************************************
/**
 * \brief Converts the given double into a string with fps
 **/
//***********************************************************
std::string convertDoubleToString(double dTime,  double fps)
{
    timeval tv = convertDoubleToTimeVal(dTime);
    return convertTimeValToString(tv, fps);
}


//***********************************************************
/**
 * \brief Converts a double to a TimeCode
 **/
//***********************************************************
int64_t convertDoubleToTimeCode(double dTime, int fps)
{
	timeval tv = convertDoubleToTimeVal(dTime);
    return convertTimeValToTimeCode(tv, fps);
}

//***********************************************************
/**
 * \brief Gets the current time as a double
 **/
//***********************************************************
double getTime()
{
    return (double)getUsecTime() / (double)1e6;
}

/**
 * @brief Gets the number of microseconds since the epoch
 */
uint64_t getUsecTime()
{
    //Get the number of microseconds right now
    auto now = std::chrono::system_clock::now();
    auto now_us = std::chrono::time_point_cast<std::chrono::microseconds>(now);
    auto useconds = now_us.time_since_epoch();

    return useconds.count();
}

/**
 * \brief Returns the current time as utc with 2^16 sub second steps
 **/
uint64_t getTimestamp()
{
    ObjectId id;
    id.m_value = 0;

    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto now_seconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
    auto mseconds = now_ms.time_since_epoch();
    auto seconds = now_seconds.time_since_epoch();
    id.m_utc  = seconds.count();

    id.m_step = (mseconds.count() - id.m_utc)/STEP_SIZE;
    return id.m_value;
}

//***********************************************************
/*!\brief Returns the current timestamp
 *
 * \return 64bit timestamp
 *
 * This is an internal function since it will eventually calculate
 * frame rate and other information
 */
//************************************************************/
uint64_t convertDoubleToTimeStamp(double dTime)
{
    return (uint64_t)(dTime * 1e8);
}

/**
 *!\brief calculates the sum of two time values
 &
 * Calcs the sum of tv1 and tv2.  Returns the sum in a timeval struct.
 * Calcs negative times properly, with the appropriate sign on both tv_sec
 * and tv_usec (these signs will match unless one of them is 0).
 * NOTE: both abs(tv_usec)'s must be < 1000000 (ie, normal timeval format)
 *
 * borrowed copiously from vrpn (thanks Russell )
 **/
/*{
    timeval tvSum = tv1;

    tvSum.tv_sec += tv2.tv_sec;
    tvSum.tv_usec += tv2.tv_usec;

    // do borrows, etc to get the time the way i want it: both signs the same,
    // and abs(usec) less than 1e6
    if (tvSum.tv_sec > 0) {
        if (tvSum.tv_usec < 0) {
            tvSum.tv_sec--;
            tvSum.tv_usec += 1000000;
        } else if (tvSum.tv_usec >= 1000000) {
            tvSum.tv_sec++;
            tvSum.tv_usec -= 1000000;
        }
    } else if (tvSum.tv_sec < 0) {
        if (tvSum.tv_usec > 0) {
            tvSum.tv_sec++;
            tvSum.tv_usec -= 1000000;
        } else if (tvSum.tv_usec <= -1000000) {
            tvSum.tv_sec--;
            tvSum.tv_usec += 1000000;
        }
    } else {
        // == 0, so just adjust usec
        if (tvSum.tv_usec >= 1000000) {
            tvSum.tv_sec++;
            tvSum.tv_usec -= 1000000;
        } else if (tvSum.tv_usec <= -1000000) {
            tvSum.tv_sec--;
            tvSum.tv_usec += 1000000;
        }
    }

    return tvSum;
}*/

/**
 *!\brief perform normalization of a timeval
 * XXX this still needs to be checked for errors if the timeval
 * or the rate is negative
 **/
/*static inline void timevalNormalizeInPlace(timeval& in_tv)
{
    const long div_77777 = (in_tv.tv_usec / 1000000);
    in_tv.tv_sec += div_77777;
    in_tv.tv_usec -= (div_77777 * 1000000);
}*/

/**
 *!\brief perform normalization of a timeval
 **/
/*timeval TimevalNormalize(const timeval& in_tv)
{
    timeval out_tv = in_tv;
    timevalNormalizeInPlace(out_tv);
    return out_tv;
}*/

/**
 *!\brief Calcs the diff between tv1 and tv2.
 *
 *\return the diff in a timeval struct.
 * Calcs negative times properly, with the appropriate sign on both tv_sec
 * and tv_usec (these signs will match unless one of them is 0)
 **/
/*timeval TimevalDiff(const timeval& tv1, const timeval& tv2)
{
    timeval tv;

    tv.tv_sec = -tv2.tv_sec;
    tv.tv_usec = -tv2.tv_usec;

    return TimevalSum(tv1, tv);
}*/

/**
 *!\brief multiplies the timevale by the given structure
 **/
/*timeval TimevalScale(const timeval& tv, double scale)
{
    timeval result;
    result.tv_sec = (long)(tv.tv_sec * scale);
    result.tv_usec =
        (long)(tv.tv_usec * scale + fmod(tv.tv_sec * scale, 1.0) * 1000000.0);
    timevalNormalizeInPlace(result);
    return result;
}*/

/**
 *!\brief compares two time values
 *\return 1 if tv1 is greater than tv2;  0 otherwise
 **/
/*bool TimevalGreater(const timeval& tv1, const timeval& tv2)
{
    if (tv1.tv_sec > tv2.tv_sec) {
        return 1;
    }
    if ((tv1.tv_sec == tv2.tv_sec) && (tv1.tv_usec > tv2.tv_usec)) {
        return 1;
    }
    return 0;
}*/

/**
 *!\brief checks if two timevals are equal
 *
 *\return 1 if tv1 is equal to tv2; 0 otherwise
 **/
/*bool TimevalEqual(const timeval& tv1, const timeval& tv2)
{
	return (tv1.tv_sec == tv2.tv_sec && tv1.tv_usec == tv2.tv_usec);
}*/

/**
 *\brief converts the ObjectId time into a double. the m_id field is lost
 **/
/*double convertObjectIdTimeToDouble(ObjectId id)
{
    return (double)id.m_utc + (double) (id.m_step * STEP_SIZE) / TIMER_STEP;
}*/

/**
 *\brief converts a double time to an ObjectId time. The m_id field is set to 0
 *\param [in] value time value to set to the object
 *\return ObjectId with the m_utc and m_step set appropriately.
 *
 * **NOTE** - There is a potential for a rounding error to have the step off by 1
 **/
/*ObjectId convertDoubleToObjectIdTime(double value)
{
    ObjectId result;
    uint64_t utc = value;
    uint64_t TS=TIMER_STEP;
    uint64_t SZ = STEP_SIZE;
    uint64_t step = (value-utc)*(double)TS/SZ;
    result.m_id  = 0;
    result.m_utc = utc;
    result.m_step = step;
    return result;
}*/

/**
 * @brief This function will convert millisecond time to our standard frameTime timestep
 *
 * @param usecs Microseconds since the epoch
 * @param fps The current fps of the camera
 *
 * @return
 */
/*uint64_t convertUsecsToFrameTime(uint64_t usecs, double fps)
{
    FrameTime time;
    time.m_utc = usecs / 1e6;
    unsigned subseconds = round((usecs - time.m_utc * 1e6) / 1.0e6);
    time.m_step = UINT32_MAX / fps * subseconds;
    return time.m_value;
}*/


/**
 * @brief This function will convert usecs time to standard date time
 *
 * @param usecs Microseconds since the epoch
 *
 * @return
 */
/*std::string convertUsecsToDate(uint64_t usecs)
{
    // using namespace std;
    // using namespace std::chrono;
    // typedef duration<int, ratio<86400>> days;
    // char fill = os.fill();
    // os.fill('0');
    // auto d = duration_cast<days>(usecs);
    // usecs -= d;
    // auto h = duration_cast<hours>(usecs);
    // usecs -= h;
    // auto m = duration_cast<minutes>(usecs);
    // usecs -= m;
    // auto s = duration_cast<seconds>(usecs);
    // os << setw(2) << d.count() << "d:"
    //    << setw(2) << h.count() << "h:"
    //    << setw(2) << m.count() << "m:"
    //    << setw(2) << s.count() << 's';
    // os.fill(fill);
    return "";
 
}*/

/**
 * \brief Sleeps for the given amount of time
 *
 * \param [in] double time to sleep in seconds (supports millisecond resolution)
 **/
void sleep(double time)
{
    if(time <= 0) {
        return;
    }

    size_t msec = time * 1e3;
    std::this_thread::sleep_for(std::chrono::milliseconds(msec));
}

/**
 * \brief Returns the current date and time as a string
 **/
std::string getDateAsString() {
  auto now = std::chrono::system_clock::now();
  auto as_time_t = std::chrono::system_clock::to_time_t(now);

  char some_buffer[1000];
  struct tm tm;
  struct tm * timeInfo = localtime( &as_time_t );

#ifdef _WIN32
  if (::gmtime_s(&tm, &as_time_t))
#else
  if (::gmtime_r(&as_time_t, &tm))
#endif
    if (std::strftime(some_buffer, sizeof(some_buffer), "%F_%H:%M:%S%p", timeInfo))
      return std::string{some_buffer};
  throw std::runtime_error("Failed to get current date as string");
}

//***********************************************************
/*!\brief functional test for this class
 */
//***********************************************************
JsonBox::Value testTimer(bool printFlag, bool assertFlag)
{
    //create file to redirect output
    timerTest.open("TimerTest.log");
    timerJson.open("timerTest.json");

    JsonBox::Value resultString;
    Timer timer;

    double timeVariance = (double) 2.0 / 1e3;
    double currTime = getTime();

    if (printFlag) {
        timerTest << "Regression test for Timer class" << endl;
    }

    timer.setFPS(STARTING_FPS);

    //Check ObjectId size
    objectIDSizeTest(printFlag, assertFlag);

    //Test sleep time
    timer.start();
    double delayTime = 10.0 / 1e3;
    sleep(delayTime);
    double sleepElapsed = timer.elapsed();

    sleepTest(resultString, delayTime, sleepElapsed, timeVariance, 
    			printFlag, assertFlag, 1);

    //Test a longer sleep time
    timer.start();
   	delayTime = 1.0;
   	sleep(delayTime);
   	sleepElapsed = timer.elapsed();

	sleepTest(resultString, delayTime, sleepElapsed, timeVariance, 
				printFlag, assertFlag, 2);
 
   	//Get current timestamp
    int64_t tstamp = convertDoubleToTimeStamp(currTime);

    if (printFlag) {
        timerTest << "Current timestamp: " << tstamp << endl;
    }

    resultString["current timestamp"] = (double) tstamp;

    //Get elapsed timestamp
    timer.start();
    double tElapsed = timer.elapsed();
    int64_t tstampElapsed = convertDoubleToTimeStamp(tElapsed);

    if (printFlag) {
    	timerTest << "Timer since second start (can be 0 if <1msec)" 
    				<< tstampElapsed << endl;
    }

    resultString["updated timestamp"] = (double) tstampElapsed;

    //Get SMPTE time
    SMPTETime currsmpte = convertDoubleToSMPTE(currTime, timer.getFPS());

    if (printFlag) {
    	printf( "SMPTETime: %2d:%2d:%2d, frame: %d\n"
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
    	timerTest << "String time: " << timeStr.c_str() << endl;
    }

    resultString["string time"] = timeStr;

    //Get time code
    timeval tempCode = convertDoubleToTimeVal(currTime);
    int64_t timeCode = convertTimeValToTimeCode(tempCode, timer.getFPS());

    if (printFlag) {
    	timerTest << "Time code: " << timeCode << endl;
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

    return resultString;
}

void objectIDSizeTest(bool printFlag, bool assertFlag) 
{
    if (sizeof(uint64_t) != sizeof(ObjectId)) {
        if (printFlag) {
            timerTest << "size uint64_t: " << sizeof(uint64_t) << "!= ObjectId: " << sizeof(ObjectId) << "!\n"
                      << "Check variable alignment in ObjectId structure." << std::endl;

            cout << "Timer test failed. See TimerTest.log" << endl;
        }

        if (assertFlag) {
            assert(false);
        };

        return;
    }
}

void sleepTest(JsonBox::Value& resultString, double delayTime, double sleepElapsed,
				double timeVariance, bool printFlag, bool assertFlag, int testno) 
{
	string currTest = "sleep time " + std::to_string(testno);

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
    		timerTest << currTest << ": " << std::fixed << delayTime << "failed: " 
    					<< sleepElapsed << ">" << delayTime + timeVariance << std::endl;
    		cout << "Timer test failed. See TimerTest.log" << endl;
    	}

    	if (assertFlag) {
    		assert(false);
    	};

    	resultString[currTest]["status"] = "fail";
    	return;
    }

    resultString[currTest]["status"] = "pass";
}

}
