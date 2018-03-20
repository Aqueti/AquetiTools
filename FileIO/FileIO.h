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

   //File system functions
   bool     create_directory( std::string name );
   bool     exists( std::string name);
   bool     remove( std::string name);
   uint64_t remove_all( std::string name);
   bool     is_directory( std::string name );
   bool     current_path(std::string path);                //Set the current path
   std::string current_path();                             //Get the current path
}
}
