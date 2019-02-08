/*****************************
 * \brief Verifes storage system capabilities
 * Copyright Aqueti 2018
 *****************************/
#include <iostream>
#include <thread>
#include <mutex>

#include <Timer.h>
#include <FileIO.h>
#include <StringTools.h>

#define MEGABYTE (1024*1024)

#define H264_HIGH_BANDWIDTH  300/19               //!< per microcamera bandwidth for hi-res H264
#define DEFAULT_BLOCK_SIZE  4*MEGABYTE
#define DEFAULT_BLOCKS_PER_CONTAINER 64           //!< Number of blocks per contains
#define DEFAULT_FRAME_RATE 30
//sdf #define DEFAULT_INPUT_STREAMS 19
#define DEFAULT_INPUT_STREAMS 2
#define DEFAULT_OUTPUT_STREAMS 1
#define DEFAULT_MCAMS_PER_OUTPUT_STREAM
#define DEFAULT_MCAMS_PER_DIR 1
#define DEFAULT_OUTPUT_LATENCY 10

std::mutex g_writeCountMutex;
uint64_t   g_writeCount = 0;
uint64_t   g_writeBytes = 0;
uint64_t   g_inputStreams = 1;

std::mutex g_readCountMutex;
uint64_t   g_readCount  = 0;
uint64_t   g_readBytes = 0;

std::string g_basePath = "./test";
uint64_t    g_filesPerDir = 1000;
uint64_t    g_fileSize = 4*MEGABYTE;

std::mutex  g_maxMapMutex;
std::mutex  g_minMapMutex;
std::map<std::string, uint64_t> g_maxStreamMap;
std::map<std::string, uint64_t> g_minStreamMap;

std::vector<std::string> g_names;

void statusFunction(double interval, bool *running )	
{ 
   uint64_t count = 0;
   uint64_t prevWriteCount = g_writeCount;
   uint64_t prevWriteBytes = g_writeBytes;
   uint64_t prevReadCount  = g_readCount;
   uint64_t prevReadBytes  = g_readBytes;

   atl::Timer timer; 
   while(*running) {
      if( timer.elapsed() > interval ) {
	 double interval = timer.elapsed();
         timer.start();
	 uint64_t myWriteCount = g_writeCount - prevWriteCount;
	 uint64_t myWriteBytes = g_writeBytes - prevWriteBytes;
	 double   writeRate = (double)myWriteBytes*8.0/interval/(double)MEGABYTE;
	 uint64_t myReadCount = g_readCount - prevReadCount;
	 uint64_t myReadBytes = g_readBytes - prevReadBytes;
	 double readRate = (double)myReadBytes*8.0/interval/(double)MEGABYTE;



         std::cout << count <<" "<< atl::getDateAsString()
		   <<" writes: "<<myWriteCount<<" files/"<<myWriteBytes<<" bytes/"<<writeRate<<" Mbps,"
		   <<" reads: "<<myReadCount<<" files/"<<myReadBytes<<" bytes/"<<readRate<<" Mbps,"
		   <<" total writes: "<<g_writeCount<<" files/"<<g_writeBytes<<" bytes,"
		   <<" total reads: "<<g_readCount<<" files/"<<g_readBytes<<"bytes" 
		   <<std::endl;

        prevWriteCount = g_writeCount;
        prevWriteBytes = g_writeBytes;
        prevReadCount  = g_readCount;
        prevReadBytes  = g_readBytes;

         count++;
      }
      else {
         atl::sleep(.01);
      }
   }
}

/**
 * \brief get the max index all uCams have
 **/
uint64_t getMaxIndex()
{
   uint64_t minVal = UINT64_MAX;

   for( auto it:g_names ) {
      if( g_maxStreamMap[it] < minVal ) {
         minVal = g_maxStreamMap[it];
      }
   }

   return minVal;
}

/**
 * \brief get the min index all uCams have
 **/
uint64_t getMinIndex()
{
   uint64_t maxVal = 0;

   for( auto it:g_names ) {
      if( g_minStreamMap[it] > maxVal ) {
         maxVal = g_minStreamMap[it];
      }
   }

   return maxVal;
}

/**
 * \brief function for statistics on writtend data
 * \param [in] bytes number of bytes written to disk
 *
 * This function should be called whenever a single file is written
 * to disk. It will increment the number of files written and the
 * total number of bytes written based on the provided parameter.
 **/
void incrementWrite(uint64_t bytes)
{
   std::lock_guard<std::mutex> lock( g_writeCountMutex );
   g_writeBytes += bytes;
   g_writeCount++;
}

/**
 * \brief function for statistics on writtend data
 * \param [in] bytes number of bytes written to disk
 *
 * This function should be called whenever a single file is read
 * from disk. It will increment the number of files read and the
 * total number of bytes read based on the provided parameter.
 **/
void incrementRead(uint64_t bytes)
{
   std::lock_guard<std::mutex> lock( g_writeCountMutex );
   g_readBytes += bytes;
   g_readCount++;
}

/**
 * \brief Generates a filename given a camera name and file count
 * \param [in] name microcamera name
 * \param [in] count frame number for the specificed microcamera
 * \return returns the fully defined path and filename for the file
 **/
std::string generateFilename( std::string name, uint64_t count ) 
{
   uint64_t dirIndex = count / g_filesPerDir;

   //Create my path
   std::string filename = g_basePath;
   filename.append("/");
   filename.append(name);
   filename.append("/");
   filename.append(std::to_string(dirIndex));
   filename.append("/");
   filename.append(std::to_string(count));

   return filename;
}

/**
 * \brief Storage Function
 **/
void writeFunction( std::string name, double rate, bool * running )
{
   //Add ourselves to teh global map
   {
      std::lock_guard<std::mutex> minLock (g_minMapMutex);
      std::lock_guard<std::mutex> maxLock (g_maxMapMutex);

      g_maxStreamMap.insert( std::make_pair(name,0));
      g_minStreamMap.insert( std::make_pair(name,0));
   }
   std::cout << "Starting write function "<<name<<std::endl;
   atl::Timer timer;

   uint64_t previousTime = 0;
   uint64_t myCount = 0;
   std::string myPath;
   double myFreq = 1.0/rate;

   //Create my file
   char * data = new char[g_fileSize];

   std::string filename = generateFilename( name, myCount );
   std::vector<std::string> pathVect = atl::stringParser( filename, "/" );

   std::string path;

   //Delete all subthreads
   for( auto it = begin(pathVect); it+1 != end( pathVect); ++it ) {
      path.append(*it);
      std::cout << "adding "<<path<<std::endl;
      if( !atl::filesystem::is_directory( *it )) {
         atl::filesystem::create_directory( path );
      }
      path.append("/");
       
   }

   //Main loop
   while( *running )
   {
      std::string filename = generateFilename( name, myCount);

      //Check timer. Wait until it's time to insert, then add
      while( timer.elapsed() < previousTime+myFreq) {
        std::this_thread::sleep_for( std::chrono::milliseconds(1));
      }

      timer.start();

      //Open the file
      FILE * fptr = NULL;
      fptr = fopen( filename.c_str(), "wb" );
      if( fptr == NULL ) {
         std::cout << "Failed to open file "<<filename<<std::endl;
         continue;
      }

      //Write data and close the file
      fwrite( data, 1, g_fileSize, fptr );
      fclose(fptr);
      fptr = NULL;

      //Update global tracking
      incrementWrite( g_fileSize );
      myCount++;

      //Update max value in maxMap.
      {
         std::lock_guard<std::mutex> maxLock (g_maxMapMutex);
         g_maxStreamMap[name] = myCount;
      }
   }

   std::cout << "Closing write thread "<<name<<std::endl;

   //Delete data
   delete [] data;
}



void readFunction( uint64_t startOffset
      , double rate
      , bool * running 
      )
{
   atl::Timer timer;
   uint64_t index = 0;
   uint64_t previousTime = 0;
   double myFreq = 1.0/rate;

   //Create my file for reading
   char * data = new char[g_fileSize];

   //Main loop
   while( *running )
   {
      uint64_t maxIndex = getMaxIndex();

      if( maxIndex < startOffset ) {
         continue;
      }


      for( auto it : g_names ) {
         //get filename
         std::cout<<"Generating filename for "<<it<<","<<std::to_string(index)<<std::endl;
         std::string filename = generateFilename( it, index);
	
         while( timer.elapsed() < previousTime+myFreq) {
            atl::sleep(.001);

	    if( ! *running ) {
	       break;
	    }
	 }
         //Write the file
         FILE * fptr = NULL;
         fptr = fopen( filename.c_str(), "rb" );
         if( fptr == NULL ) {
            std::cout << "Failed to open file for reading "<<filename<<std::endl;
            continue;
         }

         //Read data
         uint64_t bytes = fread( data, 1, g_fileSize, fptr );
         fclose(fptr);
         fptr = NULL;

         timer.start();

	 if( bytes != g_fileSize )  {
            std::cout <<"ERROR: read "<<bytes<<"/"<<g_fileSize<<" bytes from "<<filename<<std::endl;
         }

         //Update global tracking
         incrementRead( g_fileSize );
      }

      index++;
      
   }

   //Delete data
   delete [] data;
}
   

/**
 * \brief Main function
 **/
int main( int argc, char * argv[] )
{
    //General params
    uint64_t streamRate         = H264_HIGH_BANDWIDTH *1024*1024/8;

    //Input params
    double   inputFrameRate       = DEFAULT_FRAME_RATE;

    //Storage params
    uint64_t blocksPerContainer = DEFAULT_BLOCKS_PER_CONTAINER;
    uint64_t blockSize          = DEFAULT_BLOCK_SIZE;
    uint64_t uCamsPerDirectory  = DEFAULT_MCAMS_PER_DIR;

    //Playback params
    uint64_t outputStreams = DEFAULT_OUTPUT_STREAMS;
    uint64_t streamsPerOutputStream = g_inputStreams;
    uint64_t readOffset = DEFAULT_OUTPUT_LATENCY;                    //!< How far behind write to start
    double   outputFrameRate     = DEFAULT_FRAME_RATE;

    /////////////////////////////////////////////
    //Read in command line options
    /////////////////////////////////////////////


    //Calculate values
    uint64_t totalWriteBandwith = streamRate*g_inputStreams;
    uint64_t readBandwidthPerOutputStream = streamsPerOutputStream*streamRate;
    uint64_t totalReadBandwidth = readBandwidthPerOutputStream * outputStreams;

    g_fileSize             = blocksPerContainer * blockSize;
    double   filesPerSecPerStream = g_fileSize/streamRate;
    double   totalFilesPerSec     = g_inputStreams * filesPerSecPerStream; 

    //Print assumptions
    std::cout <<"Aqueti Storage Validation Tool"<<std::endl;
    std::cout <<"Input:"<<std::endl;
    std::cout <<"\tstreams:    "<<g_inputStreams<<std::endl;
    std::cout <<"\tframe rate: "<<inputFrameRate<<std::endl;
    std::cout <<"\tStream bandwidth: "<<(double)streamRate/(double)MEGABYTE<<" Mbps"<<std::endl;
    std::cout <<"\tTotal write bandwidth: "<<totalWriteBandwith<<" Mbps"<<std::endl;
    std::cout <<"\tFiles per directory: "<<g_filesPerDir<<std::endl;

    std::cout <<"Output:"<<std::endl;
    std::cout <<"\tstreams: "<<outputStreams<<std::endl;
    std::cout <<"\tinput streams required: "<<streamsPerOutputStream<<std::endl;
    std::cout <<"\tread bandwidth per stream:"<< readBandwidthPerOutputStream<<std::endl;
    std::cout <<"\ttotal read bandwidth per stream:"<<totalReadBandwidth<<std::endl;
    std::cout <<"\tframes behind live to start: "<<readOffset<<std::endl;
    
    std::cout <<"File system info"<<std::endl;
    std::cout <<"\tStorage Path:"<< g_basePath<<std::endl;
    std::cout <<"\tFile Size:"<<g_fileSize<<std::endl;
    std::cout <<"\tFiles per second per stream: "<<filesPerSecPerStream<<std::endl;
    std::cout <<"\tTotal files per second: "<<totalFilesPerSec<<std::endl;
    std::cout <<"\tuCams per directory: "<<uCamsPerDirectory<<std::endl;

    bool running = true;
    std::vector<std::thread> writeVect;
    std::vector<std::thread> readVect;

    bool result = true;
    if( !atl::filesystem::is_directory(g_basePath)) { 
      result = atl::filesystem::create_directory(g_basePath);
    }

    if( !result ) {
       std::cout << "Base Storage directory "<<g_basePath<<" does not exist and unable to create"<<std::endl;
    }

    //Create write threads - one per input stream
    for( uint64_t i=0; i < g_inputStreams; i++ ) {
       std::string name = "uCam_";
       name.append( std::to_string(i));
    
       writeVect.push_back( std::thread( writeFunction
                       , name
		       , inputFrameRate
		       , &running
		       ));
       g_names.push_back(name);
    }


    //Create read threads - one for each output stream
    for( uint64_t i = 0; i < outputStreams; i++ ) {
       std::string name = "render_";
       name.append( std::to_string(i));

       readVect.push_back( std::thread( readFunction 
			 , readOffset
			 , outputFrameRate
			 , &running
			 ));
    }
			 

    //Create status thread. This prints the output
    std::thread statusThread = std::thread(statusFunction, 1.0, &running);

    atl::sleep(100.0);

    std::cout <<"Setting running to false!"<<std::endl;
    running = false;

    statusThread.join();

    //Delete all subthreads
    for( auto it = begin(writeVect); it != end( writeVect); ++it ) {
       it->join();
    }

    //Delete all subthreads
    for( auto it = begin(readVect); it != end( readVect); ++it ) {
       it->join();
    }
 

   
}
