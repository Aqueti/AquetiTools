/**
 * \file MongoAPITest.cpp
 *
 * Created on: June 19, 2017
 * Author: Steve Feller/Nils Persson
 **/
#include <vector>
#include <thread>
#include <JsonBox.h>

#include "mongoapi/mongoapi.h"
#include <Timer.h>

/**
 * \brief Thread to write multiple records to a databasea
 * \param [in] start initial number to write
 * \param [in] count number of items to write
 * \param [in] tid number of the thread
 * \param [in] uri ip address and port of the database server
 * \param [in] name name of the database to use
 *
 * This function writes count records to a database. The first record calculated
 * as the thread id * the number of records per 
 **/
void writeThread( int start, int count , unsigned int tid, std::string uri, std::string name ) {
   mongoapi::MongoInterface mi(uri);
   bool connected = mi.connect("test");
   if(!connected){
      std::cout << "Thread "<<tid<<" with start "<<start<<" unable to connect. Exiting" << std::endl;
      return;
   }

   //Create record
   JsonBox::Value value;
   value["tid"].setInteger(tid);
   value["type"].setString("test");

   //Loop through and write records
   for( int i = 0; i < count; i++ ) {
      value["id"].setInteger( start+i );
      mi.insert("test", value );
   }
}

/**
 * \brief Main function
 **/
int main(int argc, char * argv[] ) 
{
   unsigned int threadCount = 100;
   int writesPerThread = 10000;
   std::string uri = "127.0.0.1:27017";

   atl::Timer timer;

   //Spawn n write threads that each write m values
   std::vector<std::thread> threadVect;

   for( unsigned int i = 0; i < threadCount; i ++ ) {
      threadVect.push_back( std::thread( writeThread, i*writesPerThread, writesPerThread, i, uri, "test" ));
   }

   //Wait for all threads to complete
   for( auto it = threadVect.begin(); it != threadVect.end(); it++ ) {
      it->join();
   }

   size_t writeCount = threadCount * writesPerThread;
   double elapsed = timer.elapsed();
   double rate = writeCount / elapsed;

   std::cout << writeCount << " records written in "<<elapsed<<" seconds ("<<rate<<"fps)"<<std::endl;
}
