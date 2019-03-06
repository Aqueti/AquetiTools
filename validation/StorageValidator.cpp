/*****************************
 * \brief Verifes storage system capabilities
 * Copyright Aqueti 2018
 *****************************/
#include <iostream>
#include <thread>
#include <mutex>
#include <cstring>

#include <Timer.h>
#include <FileIO.h>
#include <StringTools.h>

#define MEGABYTE (1024*1024)

#define H264_HIGH_BANDWIDTH (300.0*MEGABYTE/19.0)          //!< per microcamera bandwidth for hi-res H264
#define DEFAULT_BLOCK_SIZE  4*MEGABYTE
#define DEFAULT_BLOCKS_PER_CONTAINER 64           //!< Number of blocks per contains
#define DEFAULT_INPUT_STREAMS 2
#define DEFAULT_OUTPUT_STREAMS 1
#define DEFAULT_MCAMS_PER_OUTPUT_STREAM
#define DEFAULT_MCAMS_PER_DIR 1
#define DEFAULT_OUTPUT_LATENCY 100


std::mutex g_writeCountMutex;
uint64_t   g_writeCount   = 0;
uint64_t   g_writeBytes   = 0;
int64_t    g_inputStreams = 1;
double     g_streamRate   = H264_HIGH_BANDWIDTH;
double     g_ifps         = 30;
double     g_filesPerSecPerStream  = 0;
double     g_totalWriteFilesPerSec = 0;
double     g_totalReadFilesPerSec  = 0;
double     g_totalRmFilesPerSec    = 0;
double     g_streamSecPerFile ;

std::mutex g_readCountMutex;
uint64_t   g_readCount     = 0;
uint64_t   g_readBytes     = 0;
uint64_t   g_outputStreams = DEFAULT_OUTPUT_STREAMS;
double     g_ofps          = 30;
uint64_t   g_readOffset    = 1100;          //!< How many files behind when reads start
uint64_t   g_readGap       = 10;           //!< How close to live we can get
double     g_pauseDur      = 60;            //!< Default pause duration

std::string g_basePath = "./test";
bool        g_rmdir       = false;
uint64_t    g_filesPerDir = 1000;
uint64_t    g_fileSize    = 4*MEGABYTE;
double      g_maxUtil     = 95;
double      g_targetUtil  = 90;

uint64_t    g_removeCount = 0;
uint64_t    g_statusInterval = 10;

std::mutex  g_maxMapMutex;
std::mutex  g_minMapMutex;
std::map<std::string, uint64_t> g_maxStreamMap;
std::map<std::string, uint64_t> g_minStreamMap;

std::vector<std::string> g_names;

/**
 * \brief Thread function that prints application statistics
 * \param [in] interval time in seconds to print output
 * \param [in] running pointer to a boolean that terminal processing loop
 **/
void statusFunction(double interval, bool *running )   
{ 
   uint64_t count = 0;
   uint64_t prevWriteCount  = g_writeCount;
   uint64_t prevWriteBytes  = g_writeBytes;
   uint64_t prevReadCount   = g_readCount;
   uint64_t prevReadBytes   = g_readBytes;
   uint64_t prevRemoveCount = g_removeCount;

   atl::Timer timer; 
   while(*running) {
      if( timer.elapsed() > interval ) {
         double interval = timer.elapsed();
         timer.start();
         int64_t myWriteCount = g_writeCount - prevWriteCount;
         uint64_t myWriteBytes = g_writeBytes - prevWriteBytes;
         double   writeRate = (double)myWriteBytes*8.0/interval/(double)MEGABYTE;
         uint64_t myReadCount = g_readCount - prevReadCount;
         uint64_t myReadBytes = g_readBytes - prevReadBytes;
         double   readRate = (double)myReadBytes*8.0/interval/(double)MEGABYTE;
         uint64_t myRemoveCount = g_removeCount - prevRemoveCount;
         

         std::cout << count <<" "<< atl::getDateAsString()
         <<" writes: "<<myWriteCount<<" files/"<<myWriteBytes<<" bytes/"<<writeRate<<" Mbps,"
         <<" reads: "<<myReadCount<<" files/"<<myReadBytes<<" bytes/"<<readRate<<" Mbps,"
         <<" total writes: "<<g_writeCount<<" files/"<<g_writeBytes/(double)MEGABYTE<<" bytes,"
         <<" total reads: "<<g_readCount<<" files/"<<g_readBytes/(double)MEGABYTE<<" MB," 
         <<" removeCount: "<<myRemoveCount<<" total removed: "<<g_removeCount
         <<" util: "<<atl::filesystem::getUtilization(g_basePath)*100.0
         <<std::endl;

         if(( myWriteCount < (int64_t)(g_inputStreams*interval/g_streamSecPerFile )-g_inputStreams)||
            ( myWriteCount > (int64_t)(g_inputStreams*interval/g_streamSecPerFile )+g_inputStreams))
         {
            std::cout << "ERROR: Wrote "<<myWriteCount<<" of "<< g_inputStreams*interval/g_streamSecPerFile<< "files"<<std::endl;
         }
         if(( myReadCount > 0 ) &&
           (( myReadCount < (int64_t)(g_outputStreams*g_inputStreams*interval/g_streamSecPerFile )-g_outputStreams)||
            ( myReadCount > (int64_t)(g_outputStreams*g_inputStreams*interval/g_streamSecPerFile )+g_outputStreams)
           )) {
            std::cout << "ERROR: Read "<<myReadCount<<" of "<< g_inputStreams*(int64_t)(g_outputStreams*interval/g_streamSecPerFile)-g_outputStreams<< " files"<<std::endl;
         }

        prevWriteCount = g_writeCount;
        prevWriteBytes = g_writeBytes;
        prevReadCount  = g_readCount;
        prevReadBytes  = g_readBytes;
        prevRemoveCount = g_removeCount;

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
 * \brief Delete oldest data for continuous operation
 **/
void reaperFunction( bool * running ) {
   while( *running ) {
      //Wait until we're at 95%
      double util = atl::filesystem::getUtilization( g_basePath )*100.0;
      if( util > g_maxUtil ) {
//         uint64_t minVal = UINT64_MAX;

         for( auto it : g_minStreamMap ) {
            std::string name = it.first;


            std::string fname = generateFilename( name, g_minStreamMap[name]);

            bool result = atl::filesystem::remove(fname);
            if( !result) {
               std::cout << "Unable to remove "<<fname<<std::endl;
            } 
            else {
               g_minStreamMap[name] = g_minStreamMap[name] + 1;
               g_removeCount++;
            }
         }  
         atl::sleep(.1);
      }
      else {
         atl::sleep(1.0);
      }
   }
}

/**
 * \brief Storage Function
 **/
void writeFunction( std::string name, double rate, bool * running )
{
   //Add ourselves to the global map
   {
      std::lock_guard<std::mutex> minLock (g_minMapMutex);
      std::lock_guard<std::mutex> maxLock (g_maxMapMutex);

      g_maxStreamMap.insert( std::make_pair(name,0));
      g_minStreamMap.insert( std::make_pair(name,0));
   }
   std::cout << "Starting write function "<<name<<std::endl;
   atl::Timer timer;

   uint64_t myCount = 0;
   std::string myPath;
   double myFreq = rate;

   //Create my file
   char * data = new char[g_fileSize];

   std::string filename = generateFilename( name, myCount );
   std::vector<std::string> pathVect = atl::stringParser( filename, "/" );

   std::string path;

   //Delete all subthreads
   for( auto it = begin(pathVect); it+2 != end( pathVect); ++it ) {
      path.append(*it);
      //If path doesn't exist, add it
      if( !atl::filesystem::exists(path)) {
         if( !atl::filesystem::is_directory( *it )) {
            atl::filesystem::create_directory( path );
         }
      }
      path.append("/");
      atl::sleep(.1);
       
   }

   //Main loop
   while( *running )
   {

      std::string filename = generateFilename( name, myCount);

      //If its time for a new directory, create the directory
      if( !(myCount % g_filesPerDir )) {
         std::vector<std::string> path = atl::stringParser( filename, "/");
      
         std::string dirname;   
         for( uint64_t i = 0; i < path.size()-1; i++ ) {
            dirname.append(path[i]);
            dirname.append("/");
         }

         std::cout << "Creating directory: "<<dirname<<std::endl;
         atl::filesystem::create_directory( dirname );

      }

      //Check timer. Wait until it's time to insert, then add
      while( timer.elapsed() < myFreq) {
        std::this_thread::sleep_for( std::chrono::milliseconds(1));
      }

      timer.start();

      //Open the file
      FILE * fptr = NULL;
      fptr = fopen( filename.c_str(), "wb" );
      if( fptr == NULL ) {
         std::cout << "Failed to open file "<<filename<<std::endl;
      }
      else {

        //Write data and close the file
        size_t bytes = fwrite( data, 1, g_fileSize, fptr );
        if( bytes < g_fileSize ) {
           std::cout << "ERROR: wrote "<<bytes<<" of "<<g_fileSize<<"bytes to "<<filename<<std::endl;
        }
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
   }

   std::cout << "Closing write thread "<<name<<std::endl;

   //Delete data
   delete [] data;
}


/**
 * \brief Fucntions for reading files
 **/
void readFunction( uint64_t startOffset
      , double rate
      , bool * running 
      )
{
   atl::Timer timer;
   int64_t index = 0;
   double myFreq = 1/rate;

   //Create my file for reading
   char * data = new char[g_fileSize];

   //Main loop
   while( *running )
   {
      bool delayed =  false;
      uint64_t maxIndex = getMaxIndex();
      uint64_t bytesRead = 0;

      //If our max index is greater than the start offset, begin reading
      if(( maxIndex > startOffset )&&(index < (int64_t)(maxIndex - g_readGap))) {
         for( auto it : g_names ) {
            //get filename
            std::string filename = generateFilename( it, index);
     
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


            if( bytes != g_fileSize )  {
               std::cout <<"ERROR: read "<<bytes<<"/"<<g_fileSize<<" bytes from "<<filename<<std::endl;
            }
            bytesRead += bytes;

         }
         //Update global tracking
         incrementRead( bytesRead );
         index++;


      }
      else {
         delayed = true;
         if( maxIndex > startOffset ) {
            std::cout << "Within "<<g_readGap<<" frames of live. Reading paused for "<<g_pauseDur<<" seconds"<<std::endl;
         }

         atl::Timer timer;
         while(( timer.elapsed() < g_pauseDur) && *running ) {
            atl::sleep(0.1);
         }
      }

      //Wait until it's time for the next read
      while(( timer.elapsed() < myFreq )&&(*running)) {
            atl::sleep(.01);
      }

      if(!delayed && ( timer.elapsed() > myFreq *1.1 )) {
         std::cout  <<" ERROR: Read behind by "<<timer.elapsed()-myFreq<<" seconds ("<<timer.elapsed()<<">"<<myFreq<<std::endl;
      }
 
      //Restart the the timer
      timer.start();
   }

   //Delete data
   delete [] data;
}
   
void printHelp()
{
   std::cout << "Aqueti Storage Validator\n"<<std::endl;
   std::cout << "This application is used to verify that a storage solution is able to support a" <<std::endl;
   std::cout << "specific configuration without requiring a full software installation. It is "<<std::endl; 
   std::cout << "designed to work with both Windows and Linux systems."<<std::endl;
   std::cout << std::endl;
   std::cout << "This application emulates a system with a given number of input streams from"<<std::endl;
   std::cout << "individual microcameras. The characteristics and number of streams can be"<<std::endl;
   std::cout << "specified through the Input Stream parameters. The following tables specify"<<std::endl;
   std::cout << "standard settings for Aqueti cameras.\n"<<std::endl;
   std::cout << "   Products"<<std::endl;
   std::cout << "\tProduct    istreams "<<std::endl; 
   std::cout << "\tMantis70     19     "<<std::endl;
   std::cout << "\tMantis150    18     "<<std::endl;
   std::cout << "\tPathfinder    6     "<<std::endl;
   std::cout << std::endl;
   std::cout << "   Per stream bandwidth for standard video modes"<<std::endl;
   std::cout << "\tMode       Quality    fps    Bandwidth"<<std::endl;
   std::cout << "\tH264       High       30     600 Mbps"<<std::endl;
   std::cout << "\tH264       Medium     30     400 Mbps"<<std::endl;
   std::cout << "\tH264       Low        30     200 Mbps"<<std::endl;
   std::cout << "\tH265       High       30     300 Mbps"<<std::endl;
   std::cout << "\tH265       Medium     30     200 Mbps"<<std::endl;
   std::cout << "\tH265       Low        30     100 Mbps"<<std::endl;
   std::cout << std::endl;
   std::cout << "The fps field is used to scale output bandwidth based on input frame rate"<<std::endl;
   std::cout << std::endl;

   std::cout << "Options:"<<std::endl;
   std::cout << "   Setup Parameters"<<std::endl;
   std::cout << "\t-basePath <value>    directory to write data to"<<std::endl;
   std::cout << "\t-rmdir              remove output directory if it exists (default = true)"<<std::endl;
   std::cout << "   Input Parameters"<<std::endl;
   std::cout << "\t-istreams <value>    number of input streams (default = "<<g_inputStreams<<")"<<std::endl;
   std::cout << "\t-ifps <value>        frame rate of input streams in fps (default = "<<g_ifps<<")"<<std::endl;
   std::cout << "\t-streamMbps <value>  average stream bitrate in Mbps (default = "<<(double)g_streamRate/(double)MEGABYTE<<")"<<std::endl;
   std::cout << "   Storage Parameters"<<std::endl;
   std::cout << "\t-fileSize <value>    size of the files written to disk in bytes (default = "<<g_fileSize<<")"<<std::endl;
   std::cout << "\t-filesPerDir <value> number of files written into a single directory (default = "<<g_filesPerDir<<")"<<std::endl;
   std::cout << "\t-maxUtil  <value>    percent of drive utilization to begin deleting files (default = "<<g_maxUtil<<")"<<std::endl;
   std::cout << "   Output Parameters"<<std::endl;
   std::cout << "\t-ostreams <value >   number of output streams (default = "<<g_outputStreams<<")"<<std::endl;
   std::cout << "\t-ofps <value>        frame rate of output streams in fps (default = "<<g_ifps<<")"<<std::endl;
   std::cout << "\t-readOffset <value>  how many files behind input do we start reading (default = "<<g_readOffset<<")"<<std::endl;
   std::cout << "\t-minReadGap <value>  how far behind live do we stop reading data (defalt = "<<g_readGap<<")"<<std::endl;
   std::cout << "\t-interval <value>    how often statistics are written (default = "<<g_statusInterval<<")"<<std::endl;

   return;
}

/**
 * \brief Main function
 **/
int main( int argc, char * argv[] )
{
   //General params

   //Input params
   //Storage params
//   uint64_t blocksPerContainer = DEFAULT_BLOCKS_PER_CONTAINER;
//   uint64_t blockSize          = DEFAULT_BLOCK_SIZE;

   //Playback params
   uint64_t streamsPerOutputStream = g_inputStreams;

   /////////////////////////////////////////////
   //Read in command line options
   /////////////////////////////////////////////
   for( int i = 1; i < argc; i++ ) {
      if( !std::strcmp(argv[i], "-h")) {
         printHelp();
         exit(1);
      }
      else if(!std::strcmp(argv[i], "-istreams")) {
         g_inputStreams = std::atoi(argv[++i]);
         if( g_inputStreams == 0 ) {
            std::cout << "Invalid istreams value: "<<argv[i]<<std::endl;
            exit(1);
         }
      }
      else if(!std::strcmp(argv[i], "-ifps")) {
         g_ifps = std::atof(argv[++i]);
         if( g_ifps == 0 ) {
            std::cout << "Invalid ifps value: "<<argv[i]<<std::endl;
            exit(1);
         }
      }
      else if(!std::strcmp(argv[i], "-streamMbps")) {
         g_streamRate = std::atof(argv[++i]) * MEGABYTE;
         if( g_streamRate == 0 ) {
            std::cout << "Invalid streamMbps value: "<<argv[i]<<std::endl;
            exit(1);
         } 
      }
      else if(!std::strcmp(argv[i], "-fileSize")) {
         g_fileSize = std::atoi(argv[++i]);
         if( g_fileSize == 0 ) {
            std::cout << "Invalid fileSize value: "<<argv[i]<<std::endl;
            exit(1);
         } 
      }
      else if(!std::strcmp(argv[i], "-filesPerDir")) {
         g_filesPerDir = std::atoi(argv[++i]);
         if( g_filesPerDir == 0 ) {
            std::cout << "Invalid filesPerDir value: "<<argv[i]<<std::endl;
            exit(1);
         } 
      }
      else if(!std::strcmp(argv[i], "-maxUtil")) {
         g_maxUtil = std::atof(argv[++i]);
         if( g_maxUtil == 0 ) {
            std::cout << "Invalid maxUtil value: "<<argv[i]<<std::endl;
            exit(1);
         }
      }
      else if(!std::strcmp(argv[i], "-ostreams")) {
         g_outputStreams = std::atoi(argv[++i]);
         if( g_outputStreams == 0 ) {
            std::cout << "Invalid ostreams value: "<<argv[i]<<std::endl;
            exit(1);
         }
      }
      else if(!std::strcmp(argv[i], "-ofps")) {
         g_ofps = std::atof(argv[++i]);
         if( g_ofps == 0 ) {
            std::cout << "Invalid ofps value: "<<argv[i]<<std::endl;
            exit(1);
         }
      }
      else if(!std::strcmp(argv[i], "-readOffset")) {
         g_readOffset = std::atoi(argv[++i]);
         if( g_readOffset == 0 ) {
            std::cout << "Invalid readOffset value: "<<argv[i]<<std::endl;
            exit(1);
         }
      }
      else if(!std::strcmp(argv[i], "-readGap")) {
         g_readGap = std::atoi(argv[++i]);
         if( g_readOffset == 0 ) {
            std::cout << "Invalid readOffset value: "<<argv[i]<<std::endl;
            exit(1);
         }
      }
      else if(!std::strcmp(argv[i], "-rmdir")) {
         g_rmdir = true;
      }
      else if(!std::strcmp(argv[i], "-basePath")) {
         g_basePath = argv[++i];
      }
      else if(!std::strcmp(argv[i], "-interval")) {
         g_statusInterval = std::atoi(argv[++i]);
         if( g_readOffset == 0 ) {
            std::cout << "Invalid readOffset value: "<<argv[i]<<std::endl;
            exit(1);
         }
      }
      else {
         std::cout << "Invalid input parameter \""<<std::string(argv[i])<<"\". Re-run with -h to see options"<<std::endl;
         exit(1);
      }
   }


   //Calculate values
   double   totalWriteBandwith = g_streamRate*(double)g_inputStreams;
   double   readBandwidthPerOutputStream = streamsPerOutputStream*g_streamRate;
   double   totalReadBandwidth = (double)readBandwidthPerOutputStream * g_outputStreams;

//   g_fileSize              = blocksPerContainer * blockSize;
   std::cout << "streamRate: "<<g_streamRate<<", fileSize: "<<g_fileSize<<std::endl;
   g_filesPerSecPerStream  = (double)g_streamRate*g_inputStreams/(double)g_fileSize/8;
   g_totalWriteFilesPerSec = (double)g_inputStreams * g_filesPerSecPerStream; 
   g_totalReadFilesPerSec  = (double)g_outputStreams *(double) g_filesPerSecPerStream * (double)g_inputStreams; 
   g_totalRmFilesPerSec    = (double)g_inputStreams * g_filesPerSecPerStream; 
   g_streamSecPerFile = 8*g_fileSize/g_streamRate;
  

   //Print assumptions
   std::cout <<"Aqueti Storage Validation Tool"<<std::endl;
   std::cout <<"Input:"<<std::endl;
   std::cout <<"\tstreams:    "<<g_inputStreams<<std::endl;
   std::cout <<"\tframe rate: "<<g_ifps<<std::endl;
   std::cout <<"\tStream bandwidth: "<<g_streamRate/MEGABYTE<<" Mbps"<<std::endl;
   std::cout <<"\tTotal write bandwidth: "<<totalWriteBandwith/MEGABYTE<<" Mbps"<<std::endl;
   std::cout <<"\tFiles per directory: "<<g_filesPerDir<<std::endl;
   std::cout <<"\tFile interval: "<<g_streamSecPerFile<<" seconds"<<std::endl;

   std::cout <<"Output:"<<std::endl;
   std::cout <<"\tstreams: "<<g_outputStreams<<std::endl;
   std::cout <<"\tinput streams required: "<<streamsPerOutputStream<<std::endl;
   std::cout <<"\tframe rate: "<<g_ofps<<std::endl;
   std::cout <<"\tread bandwidth per stream:"<< readBandwidthPerOutputStream/MEGABYTE<<" Mbps"<<std::endl;
   std::cout <<"\ttotal read bandwidth per stream:"<<totalReadBandwidth/MEGABYTE<<" Mbps"<<std::endl;
   std::cout <<"\ttotal read files per second:"<<g_totalReadFilesPerSec<<std::endl;
   std::cout <<"\tframes behind live to start: "<<g_readOffset<<std::endl;
   
   std::cout <<"File system info"<<std::endl;
   std::cout <<"\tStorage Path:"<< g_basePath<<std::endl;
   std::cout <<"\tFile Size:"<<g_fileSize<<" bytes"<<std::endl;
   std::cout <<"\tFiles per second per stream: "<<g_filesPerSecPerStream<<std::endl;
   std::cout <<"\tTotal write files per second: "<<g_totalWriteFilesPerSec<<std::endl;
   std::cout <<"\tuCams per directory: "<<g_filesPerDir<<std::endl;
   std::cout <<"\tMax Utilization: "<<g_maxUtil<<std::endl;
   std::cout <<"\tTotal files removed per second: "<<g_totalRmFilesPerSec<<std::endl;
   std::cout <<"\tStatus update interval: "<<g_statusInterval<<std::endl;


   std::cout << "Write Rate: "<< (double)g_fileSize/(double)g_streamRate<<std::endl;

   if( g_readOffset < g_readGap ) {
      std::cout << "ERROR: Read offset must be at least "<<g_readGap<<std::endl;
      exit(1);
   }

   /////////////////////////////////////////////
   // Setup
   /////////////////////////////////////////////
   bool running = true;
   std::vector<std::thread> writeVect;
   std::vector<std::thread> readVect;

   //Check if we should remove the directory
   if( g_rmdir ) {
      std::string input;
      std::cout << "Are you sure you want to remove the "<<g_basePath<<" directory (y/n)?"<<std::endl;
      std::cin >> input;

      if( input.compare("y")) {
         std::cout << "Aborting"<<std::endl;
         exit(1);
       }
     
  
      atl::filesystem::remove_all( g_basePath );

      while( atl::filesystem::exists( g_basePath )) {
         atl::sleep(.1);
      }
      
   } 

   bool result = true;
   if( !atl::filesystem::is_directory(g_basePath)) { 
     result = atl::filesystem::create_directory(g_basePath);
   }

   if( !result ) {
      std::cout << "Base Storage directory "<<g_basePath<<" does not exist and unable to create"<<std::endl;
   }
   atl::sleep(1.0);

   //Create write threads - one per input stream
   for( int64_t i=0; i < g_inputStreams; i++ ) {
      std::string name = "uCam_";
      name.append( std::to_string(i));
   
      writeVect.push_back( std::thread( writeFunction
            , name
            , g_streamSecPerFile
//            , g_fileSize/g_streamRate
            , &running
            ));
      g_names.push_back(name);
   }


   //Create read threads - one for each output stream
   for( uint64_t i = 0; i < g_outputStreams; i++ ) {
      std::string name = "render_";
      name.append( std::to_string(i));

      readVect.push_back( std::thread( readFunction 
         , g_readOffset
         , g_ofps
         , &running
         ));
   }
         

   //Create status thread. This prints the output
   std::thread statusThread = std::thread(statusFunction, g_statusInterval, &running);

   //Create a garbage collection thread
   std::thread reaperThread = std::thread(reaperFunction, &running );

   /////////////////////////////////////////////
   // Processing loop
   /////////////////////////////////////////////
   //Wait until we're at 95%
   while( true ) {
      atl::sleep(1.0);
   }

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

   reaperThread.join();


  
}
