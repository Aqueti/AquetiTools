/**
 * \file FileIO.h
 **/
#include <sys/stat.h>
#include <string>

#pragma once

namespace atl::filesystem
{
   //File system functions
   bool     createDirectory( std::string dir, int permissions);
   bool     exists( std::string dir);
   bool     remove( std::string dir);
   uint64_t removeAll( std::string dir);
   std::string currentPath();
}
