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
 *
 * \param [in] rate to set as FPS
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
 * \return SMPTETime conversion of time code
 *
 * This is an internal function since it will eventually calculate
 * frame rate and other information
 **/
//************************************************************
SMPTETime Timer::getTimeCode()
{
    double now = getTime() + m_timeCodeOffset;

    if(m_fps == 0) {
        cout <<"Timer::getTimeCode unknown fps."<<std::endl;
    }

    return(convertDoubleToSMPTE(now, m_fps));
}


//***********************************************************
/*!\brief returns the timeCode offset to match system time with global time
 */
//************************************************************
/*void Timer::updateTimeCodeOffset(int64_t refTime)
{
    
    //Get current time
    timeval tv;
    gettimeofday(&tv, NULL);

    timeCodeOffset = refTime - convertTimeValToTimeCode( tv, fps );


    return;
}*/


//***********************************************************
/*!\brief returns the timeCode offset to match system time with global time
 *
 * \return 64-bit time code offset
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
 * \return double elapsed time
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
 *
 * \param [in] tv timeval to be converted to double
 * \return double conversion of timeval
 */
//************************************************************/
double convertTimeValToDouble(timeval tv)
{
    return (double)tv.tv_sec + (double)((int)tv.tv_usec) / double(TIMER_STEP);
}

//***********************************************************
/*!\brief converts a double to a timeval struct
 *
 * \param [in] dTime double to be converted to timeval
 * \return timeval conversion of double
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
 * \param [in] dTime double to convert
 * \param [in] fps number of frames per second (used to convert decimal time to a frame)
 * \return SMPTETime conversion of double
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
 *
 * \param [in] smpte value to be converted to time code
 * \return 64-bit time code
 */
//************************************************************/
int64_t convertSMPTEToTimeCode(SMPTETime smpte)
{
    return smpte.hour*1e6 + smpte.minute*1e4+smpte.second*1e2+smpte.frame;
}

//***********************************************************
/*!\brief converts a timeval structure to Timecode (hhmmssff)
 *
 * \param [in] tv timeval to be converted to time code
 * \param [in] fps number of frames per second (used to convert time val to SMPTE)
 * \return 64-bit time code
 **/
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
 * \return SMPTETime conversion of timeval
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
 *
 * \param [in] tv timeval to be converted to string
 * \param [in] fps number of frames per second (used to convert time val to SMPTE)
 * \return std::string conversion of timeval
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
 *
 * \param [in] dTime double to be converted to string
 * \param [in] fps number of frames per second (used to convert time val to string)
 * \return std::string conversion of double
 **/
//***********************************************************
std::string convertDoubleToString(double dTime, double fps)
{
    timeval tv = convertDoubleToTimeVal(dTime);
    return convertTimeValToString(tv, fps);
}


//***********************************************************
/**
 * \brief Converts a double to a TimeCode
 * 
 * \param [in] dTime double to be converted to time code
 * \param [in] fps number of frams per second (used to convert time val to time code)
 * \return 64-bit time code
 **/
//***********************************************************
int64_t convertDoubleToTimeCode(double dTime, double fps)
{
	timeval tv = convertDoubleToTimeVal(dTime);
    return convertTimeValToTimeCode(tv, fps);
}

//***********************************************************
/**
 * \brief Gets the current time as a double
 *
 * \return double time
 **/
//***********************************************************
double getTime()
{
    return (double)getUsecTime() / (double)1e6;
}

/**
 * @brief Gets the number of microseconds since the epoch
 *
 * \return 64-bit microsecond time
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
 *
 * \return 64-bit timestamp
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
 * \param [in] dTime double to be converted to timestamp
 * \return 64-bit timestamp
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

timeval TimevalSum(const timeval& tv1, const timeval& tv2)
{
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
}

/*
 *!\brief compares two time values
 *\return 1 if tv1 is greater than tv2;  0 otherwise
 */

bool TimevalGreater(const timeval& tv1, const timeval& tv2)
{
    if (tv1.tv_sec > tv2.tv_sec) {
        return 1;
    }
    if ((tv1.tv_sec == tv2.tv_sec) && (tv1.tv_usec > tv2.tv_usec)) {
        return 1;
    }
    return 0;
}

/*
 *!\brief perform normalization of a timeval
 * XXX this still needs to be checked for errors if the timeval
 * or the rate is negative
 *
static inline void timevalNormalizeInPlace(timeval& in_tv)
{
    const long div_77777 = (in_tv.tv_usec / 1000000);
    in_tv.tv_sec += div_77777;
    in_tv.tv_usec -= (div_77777 * 1000000);
}


 *!\brief perform normalization of a timeval
 
timeval TimevalNormalize(const timeval& in_tv)
{
    timeval out_tv = in_tv;
    timevalNormalizeInPlace(out_tv);
    return out_tv;
}


 *!\brief Calcs the diff between tv1 and tv2.
 *
 *\return the diff in a timeval struct.
 * Calcs negative times properly, with the appropriate sign on both tv_sec
 * and tv_usec (these signs will match unless one of them is 0)
 
timeval TimevalDiff(const timeval& tv1, const timeval& tv2)
{
    timeval tv;

    tv.tv_sec = -tv2.tv_sec;
    tv.tv_usec = -tv2.tv_usec;

    return TimevalSum(tv1, tv);
}


 *!\brief multiplies the timevale by the given structure
 
timeval TimevalScale(const timeval& tv, double scale)
{
    timeval result;
    result.tv_sec = (long)(tv.tv_sec * scale);
    result.tv_usec =
        (long)(tv.tv_usec * scale + fmod(tv.tv_sec * scale, 1.0) * 1000000.0);
    timevalNormalizeInPlace(result);
    return result;
}


 *!\brief checks if two timevals are equal
 *
 *\return 1 if tv1 is equal to tv2; 0 otherwise
 
bool TimevalEqual(const timeval& tv1, const timeval& tv2)
{
	return (tv1.tv_sec == tv2.tv_sec && tv1.tv_usec == tv2.tv_usec);
}

/
 *\brief converts the ObjectId time into a double. the m_id field is lost
 **/
/*double convertObjectIdTimeToDouble(ObjectId id)
{
    return (double)id.m_utc + (double) (id.m_step * STEP_SIZE) / TIMER_STEP;
}

/
 *\brief converts a double time to an ObjectId time. The m_id field is set to 0
 *\param [in] value time value to set to the object
 *\return ObjectId with the m_utc and m_step set appropriately.
 *
 * **NOTE** - There is a potential for a rounding error to have the step off by 1
 *
ObjectId convertDoubleToObjectIdTime(double value)
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
}

/
 * @brief This function will convert millisecond time to our standard frameTime timestep
 *
 * @param usecs Microseconds since the epoch
 * @param fps The current fps of the camera
 *
 * @return
 
uint64_t convertUsecsToFrameTime(uint64_t usecs, double fps)
{
    FrameTime time;
    time.m_utc = usecs / 1e6;
    unsigned subseconds = round((usecs - time.m_utc * 1e6) / 1.0e6);
    time.m_step = UINT32_MAX / fps * subseconds;
    return time.m_value;
}



 * @brief This function will convert usecs time to standard date time
 *
 * @param usecs Microseconds since the epoch
 *
 * @return
 
std::string convertUsecsToDate(uint64_t usecs)
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
 * \param [in] time double to sleep in seconds (supports millisecond resolution)
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
 *
 * \return std::string conversion of date
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

}
