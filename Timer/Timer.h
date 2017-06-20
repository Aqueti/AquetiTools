/*!\file Timer.h
 * \brief AWARE Camera Interface Wrapper
 *
 * \author S. D. Feller 2013
 *
 * Timer data formats
 *
 * SMPTETime - structure with hour, minute, second and frame fields
 * double    - represents time as UTCSeconds.utc microseconds
 * timeval   - structure with 32bit tv_sec and 32 bit tv_usec
 * int64_t   -
 *
*/
#pragma once

#ifdef __linux__
#include <sys/time.h>
#else
#include <Winsock2.h>
#endif
#include <chrono>
#include <time.h>
#include <stdint.h>
#include <string>
#include <sstream>
#include <JsonBox.h>

namespace atl
{
#define TIMER_STEP   1e6                             //!< Resolution of the timer
#define STEP_SIZE (int)((double)TIMER_STEP/65535) //!< Steps per m_step unit


using timePoint = std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds>;

/**
 * \brief Union to access both ObjectIdData and integer values
 **/
typedef struct ObjectIdStruct {
    union {
        uint64_t     m_value;
        struct {
            uint32_t m_utc;
            uint16_t m_step;
            uint16_t m_id;
        };
    };
} ObjectId;


/**
 * @brief Sets the idealized timestamp of an image
 * The format is:
 * {
 *   uint32_t seconds since epoch
 *   uint32_t step
 * }
 * where step = (2^32-1) / fps * frame#
 */
union FrameTime {
    uint64_t m_value;
    struct {
        uint32_t m_utc;
        uint32_t m_step;
    };
};

/**
 * \brief Timecode structure that represents time as hhmmssff
 *
 * http://faculty.spokanefalls.edu/InetShare/AutoWebs/steveg/SmpteMadeSimple.pdf
 **/
typedef struct {
    uint8_t hour;                            //!< Hour (0-23)
    uint8_t minute;                          //!< Minute(0-59)
    uint8_t second;                          //!< Second(0-59)
    uint8_t frame;                           //!< Frame(0-99)
} SMPTETime;

/**
 * \brief Timer class for system metrics and measuring time
 *
 **/
class Timer
{
private:
    double  m_startTime = 0;                 //!< reference time point
//        double  m_stopTime = 0;                  //!< preallocated current time
    int64_t m_timeCodeOffset = 0;            //!< offset from system time to global timecode
    double  m_fps=30;                        //!< Number of frames per second a

public:
    Timer();
    ~Timer();
    SMPTETime   getTimeCode();
    void        updateTimeCodeOffset( int64_t refTimeCode);
    int64_t     getTimeCodeOffset();
    void        setFPS( double rate );
    double      getFPS( void );

    void        start();
    double      elapsed();
};

//Support functions
double      getTime();
uint64_t    getUsecTime();
uint64_t    getTimestamp();
double      convertTimeValToDouble( timeval tv );
SMPTETime   convertTimeValToSMPTE( timeval tv, double fps );
timeval     convertDoubleToTimeVal( double dTime );
SMPTETime   convertDoubleToSMPTE( double dTime, double fps );
std::string convertTimeValToString( timeval tv, double fps );
int64_t     convertSMPTEToTimeCode( SMPTETime time );
int64_t     convertTimeValToTimeCode( timeval tv, double fps );
int64_t     convertDoubleToTimeCode( double dTime, double fps );
uint64_t    convertDoubleToTimeStamp( double dTime );

/* Not currently used in testing
timeval TimevalSum(const timeval& tv1, const timeval& tv2);
bool TimevalGreater(const timeval& tv1, const timeval& tv2);
bool TimevalEqual(const timeval& tv1, const timeval& tv2);
timeval TimevalScale(const timeval& tv, double scale);
timeval TimevalDiff(const timeval& tv1, const timeval& tv2);
timeval TimevalNormalize(const timeval& in_tv);
*/

void        sleep(double time );
double      convertObjectIdTimeToDouble(ObjectId id);
ObjectId    convertDoubleToObjectIdTime( double value );
uint64_t    convertUsecsToFrameTime(uint64_t usecs, double fps);
// std::string convertUsecsToDate(uint64_t usecs);
std::string getDateAsString();

//Functional test for the timer
JsonBox::Value testTimer(bool printFlag = true, bool assertFlag = false);

//Timer test helper functions
void sleepTest(JsonBox::Value& resultString, double delayTime, double sleepElapsed,
                double timeVariance, bool printFlag, bool assertFlag, int testno);
void objectIDSizeTest(bool printFlag, bool assertFlag, JsonBox::Value& resultString);
}

