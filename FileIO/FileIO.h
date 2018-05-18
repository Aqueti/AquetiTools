/**
 * \file FileIO.h
 **/
#pragma once

#include <sys/stat.h>
#include <string>
namespace atl
{
namespace filesystem
{
   #define MAX_PATH_LEN 2048                               //!<Maximum length for a file path
   #define CREATE_DIRECTORY_TIMEOUT 0.5                    //!<Timeout for directory creation

   //Structure that indicates available space
   struct space_info {
      uint64_t capacity  = 0;                              //Total capacity
      uint64_t free      = 0;                              //Free space
      uint64_t available = 0;                              //available space
   };

   //File system functions
   bool     create_directory( std::string name );
   bool     exists( std::string name);
   bool     remove( std::string name);
   uint64_t remove_all( std::string name);
   uint64_t file_size( std::string name );
   bool     is_directory( std::string name );
   bool     current_path(std::string path);
   std::vector<std::string>ls( std::string path );        
   std::string current_path();                             
   space_info  space( std::string path );                 

 
}
}
