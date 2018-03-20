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
   bool     create_directory( std::string dir );
   bool     exists( std::string dir);
   bool     remove( std::string dir);
   uint64_t remove_all( std::string dir);
   std::string currentPath();
}
}
