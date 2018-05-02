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
   #define MAX_PATH_LEN 2048

   //Structure that indicates available space
   struct space_info {
      uint64_t capacity  = 0;
      uint64_t free      = 0;
      uint64_t available = 0;
   };

   //File system functions
   bool     create_directory( std::string name );
   bool     exists( std::string name);
   bool     remove( std::string name);
   uint64_t remove_all( std::string name);
   bool     is_directory( std::string name );
   bool     current_path(std::string path);                //Set the current path
   std::string current_path();                             //Get the current path
   std::vector<std::string>  dir( std::string path
                                , bool recurse = false);   //Lists all files in the path
   space_info space( std::string path );
 
}
}
