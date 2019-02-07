/*****************************
 * \brief Verifes storage system capabilities
 * Copyright Aqueti 2018
 *****************************/
#include <iostream>
#include <thread>
#include <mutex>
#include <Timer.h>
#include <FileIO.h>

#define MEGABYTE (1024*1024)

#define H264_HIGH_BANDWIDTH  300/19               //!< per microcamera bandwidth for hi-res H264
#define DEFAULT_BLOCK_SIZE  4*MEGABYTE
#define DEFAULT_BLOCKS_PER_CONTAINER 64           //!< Number of blocks per contains
#define DEFAULT_FRAME_RATE 30
//sdf #define DEFAULT_INPUT_STREAMS 19
#define DEFAULT_INPUT_STREAMS 2
#define DEFAULT_OUTPUT_STREAMS 1
#define DEFAULT_MCAMS_PER_OUTPUT_STREAM
#define DEFAULT_STORAGE_PATH "./test"
#define DEFAULT_FILES_PER_DIR 1000
#define DEFAULT_MCAMS_PER_DIR 1
#define DEFAULT_OUTPUT_LATENCY 1000

std::mutex g_writeCountMutex;
uint64_t   g_writeCount = 0;
uint64_t   g_writeBytes = 0;

std::mutex g_readCountMutex;
uint64_t   g_readCount  = 0;
uint64_t   g_readBytes = 0;

std::mutex g_maxMapMutex;
std::mutex g_minMapMutex;
std::map<std::string, uint64_t> g_maxStreamMap;
std::map<std::string, uint64_t> g_minStreamMap;


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
 * \brief Storage Function
 **/
void writeFunction( std::string name, uint64_t fileSize, double rate, std::string basePath, uint64_t filesPerDir, bool * running )
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
   uint64_t dirCount = 0;
   std::string myPath;
   double myFreq = 1.0/rate;

   //Create my file
   char * data = new char[fileSize];

   //Create my path
   std::string myBasePath = basePath;
   myBasePath.append("/");
   myBasePath.append(name);

   bool result = true;
   if( !atl::filesystem::is_directory(myBasePath)) { 
      result = atl::filesystem::create_directory(myBasePath);
   }

   if( !result ) {
      std::cout << "Storage Thread "<<name<<" unable to create path: "<<myBasePath<<". Exiting"<<std::endl;
      return;
   }


   //Main loop
   while( *running )
   {
      //Check If we are at the directory limit (or first directory)
      if(!( myCount % filesPerDir )) {
	 std::cout << "Mod for "<<std::to_string(myCount)<<": "<<std::to_string(myCount % filesPerDir )<<std::endl;
         myPath = myBasePath;
         myPath.append("/");
         myPath.append(std::to_string(dirCount));

         //create directory
         if( atl::filesystem::create_directory( myPath )) {
            std::cout << "Creating directory "<<myPath<<std::endl;
            dirCount ++;
         }
         else {
            std::cout << "Error creating directory "<<myPath<<"!"<<std::endl;
            continue;
         }
      }
      

      //Create the file we are going to write to
      std::string filename = myPath;
      filename.append("/");
      filename.append(std::to_string(myCount));

      //Check timer. Wait until it's time
      while( timer.elapsed() < previousTime+myFreq) {
        std::this_thread::sleep_for( std::chrono::milliseconds(1));
      }

      timer.start();

      //Write the file
      FILE * fptr = NULL;
      fptr = fopen( filename.c_str(), "wb" );
      if( fptr == NULL ) {
         std::cout << "Failed to open file "<<filename<<std::endl;
         continue;
      }

      //Write data
      fwrite( data, 1, fileSize, fptr );
      fclose(fptr);
      fptr = NULL;

      //Update global tracking
      incrementWrite( fileSize );
      myCount++;

      //Update max value in map
      {
         std::lock_guard<std::mutex> maxLock (g_maxMapMutex);
         g_maxStreamMap[name] = myCount;
      }

      
   }

   std::cout << "Closing write thread "<<name<<std::endl;

   //Delete data
   delete [] data;
}



void readFunction( std::string name
      , uint64_t fileSize
      , double rate
      , std::string basePath
      , uint64_t filesPerDir
      , bool * running 
		, uint64_t myCount = 0
      )
{
   atl::Timer timer;
   uint64_t previousTime = 0;
   uint64_t dirCount = 0;
   std::string myPath;
   double myFreq = 1.0/rate;

   //Create my file for reading
   char * data = new char[fileSize];

   //Create my path
   std::string myBasePath = basePath;
   myBasePath.append("/");
   myBasePath.append(name);

   if( !atl::filesystem::is_directory(myBasePath)) { 
      std::cout << "Base path "<<myBasePath<<" does not exist!"<<std::endl;
      return;
	}


   //Main loop
   while( *running )
   {
      //Check If we are at the directory limit (or first directory)
      if(!( myCount % filesPerDir )) {
         myPath = myBasePath;
         myPath.append("/");
         myPath.append(std::to_string(dirCount));
			dirCount++;
      }

      if( !atl::filesystem::is_directory( myPath)) {
            std::cout << "Directory does not exist: "<<myPath<<"!"<<std::endl;
            continue;
      }
      
      //Name the file we are going to read
      std::string filename = myPath;
      myPath.append("/");
      myPath.append(std::to_string(myCount));

      //Check timer. Wait until it's time
      while( timer.elapsed() < previousTime+myFreq) {
         std::this_thread::sleep_for( std::chrono::milliseconds(1));
      }

      timer.start();

      //Write the file
      FILE * fptr = NULL;
      fptr = fopen( filename.c_str(), "rb" );
      if( fptr == NULL ) {
         std::cout << "Failed to open file for reading "<<filename<<std::endl;
         continue;
      }

      //Write data
      fread( data, 1, fileSize, fptr );
      fclose(fptr);
      fptr = NULL;

      //Update global tracking
      incrementRead( fileSize );
      myCount++;
      
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
    double inputFrameRate       = DEFAULT_FRAME_RATE;
    uint64_t inputStreams       = DEFAULT_INPUT_STREAMS;

    //Storage params
    uint64_t blocksPerContainer = DEFAULT_BLOCKS_PER_CONTAINER;
    uint64_t blockSize          = DEFAULT_BLOCK_SIZE;
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
    uint64_t readBandwidthPerOutputStream = streamsPerOutputStream*streamRate;
    uint64_t totalReadBandwidth = readBandwidthPerOutputStream * outputStreams;

    std::string baseStoragePath   = DEFAULT_STORAGE_PATH;
    uint64_t fileSize        = blocksPerContainer * blockSize;
    double   filesPerSecPerStream = fileSize/streamRate;
    double   totalFilesPerSec     = inputStreams * filesPerSecPerStream; 

    //Print assumptions
    std::cout <<"Aqueti Storage Validation Tool"<<std::endl;
    std::cout <<"Input:"<<std::endl;
    std::cout <<"\tstreams:    "<<inputStreams<<std::endl;
    std::cout <<"\tframe rate: "<<inputFrameRate<<std::endl;
    std::cout <<"\tStream bandwidth: "<<(double)streamRate/(double)MEGABYTE<<" Mbps"<<std::endl;
    std::cout <<"\tTotal write bandwidth: "<<totalWriteBandwith<<" Mbps"<<std::endl;

    std::cout <<"Output:"<<std::endl;
    std::cout <<"\tstreams: "<<outputStreams<<std::endl;
    std::cout <<"\tinput streams required: "<<streamsPerOutputStream<<std::endl;
    std::cout <<"\tread bandwidth per stream:"<< readBandwidthPerOutputStream<<std::endl;
    std::cout <<"\ttotal read bandwidth per stream:"<<totalReadBandwidth<<std::endl;
    
    std::cout <<"File system info"<<std::endl;
    std::cout <<"\tStorage Path:"<< baseStoragePath<<std::endl;
    std::cout <<"\tFile Size:"<<fileSize<<std::endl;
    std::cout <<"\tFiles per second per stream: "<<filesPerSecPerStream<<std::endl;
    std::cout <<"\tTotal files per second: "<<totalFilesPerSec<<std::endl;
    std::cout <<"\tuCams per directory: "<<uCamsPerDirectory<<std::endl;
    std::cout <<"\tFiles per directory: "<<filesPerDirectory<<std::endl;

    bool running = true;
    std::vector<std::string> names;
    std::vector<std::thread> writeVect;
    std::vector<std::thread> readVect;

    bool result = true;
    if( !atl::filesystem::is_directory(baseStoragePath)) { 
      result = atl::filesystem::create_directory(baseStoragePath);
    }

    if( !result ) {
	    std::cout << "Storage directory "<<baseStoragePath<<" does not exist and unable to create"<<std::endl;
    }

    //Create write threads
    for( uint64_t i=0; i < inputStreams; i++ ) {
       std::string name = "uCam_";
       name.append( std::to_string(i));
    
       writeVect.push_back( std::thread( writeFunction
                       , name
		       , fileSize
		       , inputFrameRate
		       , baseStoragePath
		       , filesPerDirectory
		       , &running
		       ));
    }

    //Create read threads
    
    //Create status thread. This prints the output
    std::thread statusThread = std::thread(statusFunction, 1.0, &running);

    atl::sleep(10.0);

    std::cout <<"Setting running to false!"<<std::endl;
    running = false;

    statusThread.join();

    //Delete all subthreads
    for( auto it = begin(writeVect); it != end( writeVect); ++it ) {
       it->join();
    }
 

   
}
