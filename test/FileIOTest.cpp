/**
 * \file TimerTest.cpp
 **/
#include <JsonBox.h>
#include "AquetiToolsTest.h"

namespace atl
{

JsonBox::Value testFileIO( bool printFlag, bool assertFlag  )
{
   bool rc = true;

   std::string dir1 = "test";
   std::string dir2 = "test/test2";
   std::string file1 = "test/test2/file1.txt";
   std::string rootPath = atl::filesystem::current_path();

   std::cout << "Creating test directorys:\n\t -"<<dir1<<"\n\t -"<<dir2<<std::endl;
   atl::filesystem::create_directory(dir1);
   atl::filesystem::create_directory(dir2);

   //Check for file1 before it's created
   if( atl::filesystem::file_size( file1 ) >= 0 ) {
      std::cout << "FileIO::file_size failure. Return of non-existent file >= 0" << std::endl;
      rc = false;
   }


   //Create a file to test deletion
   std::ofstream myfile;
   myfile.open(file1.c_str());
   myfile << file1 <<std::endl;
   myfile.close();

   //Check for file1 before it's created
   int64_t sz = atl::filesystem::file_size( file1 );
   if( sz != 21 ) {
      std::cout << "FileIO::file_size failure. Size of "<<sz<<" not 21" << std::endl;
      rc = false;
   }


   std::cout << "RootPath: "<<rootPath<<std::endl;

   //Got into subdir 1
   if( !atl::filesystem::current_path(dir1)) {
      std::cout << "Unable to change directory to "<<dir1<<std::endl;
      rc = false;
   }

   std::string cpath = atl::filesystem::current_path();
   std::string refPath = rootPath;
   refPath.append("/");
   refPath.append(dir1);

   //See if we're in the correct global directory
   if( cpath.compare(refPath)) {
      std::cout << "Path failure: "<<cpath<<" != "<<refPath<<std::endl;
      rc = false;
   }

   if( !atl::filesystem::current_path("..")) {
      std::cout << "Unable to change dir by .."<<std::endl;
      rc = false;
   }

   cpath = atl::filesystem::current_path();
   if( cpath.compare(rootPath)) {
      std::cout << "Current Path \"..\" failure."<<std::endl;
   }

   //Check if stuff exists
   if( atl::filesystem::exists("test/dne")) {
      std::cout << "exists function found non-existent file"<<std::endl;
      rc = false;
   }

   if( !atl::filesystem::exists(dir1)) {
      std::cout << "exists did not file directory: "<<dir1<<std::endl;
      rc = false;
   }
   if( !atl::filesystem::exists(dir2)) {
      std::cout << "exists did not directory: "<<dir2 <<std::endl;
      rc = false;
   }
   if( !atl::filesystem::exists(file1)) {
      std::cout << "exists did not file: "<<file1<<std::endl;
      rc = false;
   }

   //Check is directory functionality
   if( !atl::filesystem::is_directory( dir2 )) {
      std::cout << "is_directory failed to indentify directory: "<<dir2<<std::endl;
      rc = false;
   }
   if( atl::filesystem::is_directory( file1 )) {
      std::cout << "is_directory indentifed file as  directory: "<<file1<<std::endl;
      rc = false;
   }

   std::cout << "ERROR Expected: Failed to create directory "<<dir1<<std::endl;
   if( atl::filesystem::create_directory(dir1)) {
      std::cout << "Passed create_directory "<<dir1<<" when failure expected!"<<std::endl;
      rc = false;
   }

   //check drive space
   atl::filesystem::space_info si = atl::filesystem::space("test");
   std::cout << "SI:"<<dir1<<" capacity:"<<si.capacity<<", free: "<<si.free<<", avail:"<<si.available<<std::endl;

   //Only remove directories if all is good. Otherwise, could delete valuable things
   if( rc) 
   {
     std::cout << "ERROR Expected: Failed to remove directory:"<<std::endl;
     if( atl::filesystem::remove(dir1)) {
        std::cout << "Passed remove test when failure expected!"<<std::endl;
        rc = false;
     }

     //Remove the test directory and all below
     uint64_t count = atl::filesystem::remove_all(dir1);
     if( count != 2 ) {
        std::cout << "Expected removal of 2 directories. Received "<<count<<std::endl;
        rc = false;
     }
   }

   std::cout << "SI:"<<dir1<<" capacity:"<<si.capacity<<", free: "<<si.free<<", avail:"<<si.available<<std::endl;



   // testing touch
   std::string testFile("touch_test" + std::to_string(atl::getUsecTime()));;
   if(atl::filesystem::exists(testFile)){
	   atl::filesystem::remove(testFile);
   }

   // file doesn't exist
   int rv = atl::filesystem::touch(testFile);
   if(rv){
	   std::cerr << "error calling atl touch on nonexistant file. returned: " << rv << std::endl;
	   rc = false;
   }
   if(!atl::filesystem::exists(testFile)){
	   std::cerr << "touch failed to create file, but returned no error. " << std::endl;
	   rc = false;
   }
   uint64_t modTime;
   int r = atl::filesystem::getLastModTime(testFile, &modTime);
   std::this_thread::sleep_for(std::chrono::seconds(1));

   uint64_t beforeTime = atl::getUsecTime();
   std::this_thread::sleep_for(std::chrono::seconds(1));
   rv = atl::filesystem::touch(testFile);
   std::this_thread::sleep_for(std::chrono::seconds(1));
   uint64_t afterTime = atl::getUsecTime();
   if(rv){
	   std::cerr << "error calling atl touch on nonexistant file. returned: " << rv << std::endl;
	   rc = false;
   }

   uint64_t newModTime;
   rv = atl::filesystem::getLastModTime(testFile, &newModTime);

   if(modTime >= newModTime || newModTime < beforeTime || newModTime > afterTime){
	   std::cerr << "atl touch failed to update file modification time" << std::endl;
	   rc = false;
   }

   atl::filesystem::remove(testFile);
   
   


   JsonBox::Value result;
   result["pass"].setBoolean(rc);
   return result;
}
}
