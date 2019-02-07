/*****************************
 * \brief Verifes storage system capabilities
 * Copyright Aqueti 2018
 *****************************/
#include <iostream>
#include <thread>
//#include <Timer.h>

#define H264_HIGH_BANDWIDTH  300/19               //!< per microcamera bandwidth for hi-res H264
#define DEFAULT_BLOCKS_PER_CONTAINER 64           //!< Number of blocks per contains
#define DEFAULT_FRAME_RATE 30
#define DEFAULT_INPUT_STREAMS 19
#define DEFAULT_OUTPUT_STREAMS 1
#define DEFAULT_MCAMS_PER_OUTPUT_STREAM
#define DEFAULT_STORAGE_PATH "./test"
#define DEFAULT_FILES_PER_DIRECTORY 1000
#define DEFAULT_MCAMS_PER_DIR 1

std::mutex g_writeCountMutex;
uint64_t g_writeCount = 0;
uint64_t g_writeBytes = 0;


uint64_t readCount  = 0;
/**
 * \brief function for statistics on writtend data
 * \param [in] bytes number of bytes written to disk
 *
 * This function should be called whenever a single file is written
 * to disk. It will increment the number of files written and the
 * total number of bytes written based on the provided parameter
 **/
void incrementWrite(uint64_t bytes)
{
   std::lock_guard<std::mutex> lock( g_writeCountMutex );
   writebytes += bytes;
   writeCount++;
}

/**
 * \brief Storage Function
 **/
void storeFunction( uint64_t fileSize, double rate, std::string basePath, bool * running )
{
   static uint64_t previousTime = 0;

   //Create my file
   char * data = new char[fileSize];

   //Create my path


   

   //Main loop
   while( *running )
   {
      
      
      
   }
}

uint64_t main( int argc, char * argv[] )
{
    //General params
    uint64_t streamRate         = H264_HIGH_BANDWIDTH *1024*1024/8

    //Input params
    double inputFrameRate       = DEFAULT_FRAME_RATE;
    uint64_t inputStreams       = DEFAULT_INPUT_STREAMS;

    //Storage params
    uint64_t blocksPerContainer = DEFAULT_BLOCKS_PER_CONTAINER;
    uint64_t blockSize          = BLK_SIZE_4MB;
    uint64_t uCamsPerDirectory  = DEFAULT_MCAMS_PER_DIR;
    uint64_t filesPerDirectory  = DEFAULT_FILES_PER_DIR;


    //Playback params
    uint64_t outputStreams = DEFAULT_OUTPUT_STREAMS;
    uint64_t streamsPerOutputStream = inputStreams;

    /////////////////////////////////////////////
    //Read in command line options
    /////////////////////////////////////////////


    //Calculate values
    uint64_t totalWriteBandwith = streamRate*inputStreams;
    uint64_t readBandwidthPerOutputStream = streamsPerOutputStreams*streamRate;
    uint64_t totalReadBandwidth = readBandwidthPerOutputStream * outputStreams;

    std::string baseStoragePath   = DEFAULT_STORAGE_PATH;
    uint64_t containerSize        = blocksPerContainer * blockSize;
    double   filesPerSecPerStream = containerSize/streamRate;
    double   totalFilesPerSec     = inputStreams * filesPerSecPerStream; 

    //Print assumptions
    std::cout <<"Aqueti Storage Validation Tool"<<std::endl;
    std::cout <<"Input:"<<std::endl;
    std::cout <<"\tstreams:    "<<inputStreams<<std::endl;
    std::cout <<"\tframe rate: "<<intputFrameRate<<std::endl;
    std::cout <<"\tStream bandwidth: "<<streamMbps<<" Mbps"<<std::endl;
    std::cout <<"\tTotal write bandwidth: "<<totalWriteBandwith<<" Mbps"<<std::endl;

    std::cout <<"Output:"<<std::endl;
    std::cout <<"\tstreams: "<<outputStreams<<std::endl;
    std::cout <<"\tinput streams required: "<<streamsPerOutputStream<<std::endl;
    std::cout <<"\tread bandwidth per stream:"<< readBandwidthPerOutputStream<<std::endl;
    std::cout <<"\ttotal read bandwidth per stream:"<<totalReadBandwidth<<std::endl;
    
    std::cout <<"File system info"<<std::endl;
    std::cout <<"\tStorage Path:"<< baseStoragePath<<std::endl;
    std::cout <<"\tFile Size:"<<containerSize<<std::endl;
    std::cout <<"\tFiles per second per stream: "<<filePerSecPerStream<<std::endl;
    std::cout <<"\tTotal files per second: "<<totalFilesPerSec<<std::endl;
    std::cout <<"\tuCams per directory: "<<mCamsPerDirectory<<std::endl;
    std::cout <<"\tFiles per directory: "<<filesPerDirectory<<std::endl;


 

   
}
