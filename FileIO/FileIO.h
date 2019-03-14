/**
 * \file FileIO.h
 **/
#pragma once

#include <sys/stat.h>
#include <string>
#include <vector>

namespace atl
{
namespace filesystem
{
   #define MAX_PATH_LEN 2048                               //!<Maximum length for a file path
   const uint64_t CREATE_DIRECTORY_TIMEOUT=0.5;            //!<Timeout for directory creation

   //Structure that indicates available space
   struct space_info {
      uint64_t capacity  = 0;                              //Total capacity
      uint64_t free      = 0;                              //Free space
      uint64_t available = 0;                              //available space
   };

   //File system functions
   bool     create_directory( std::string name, bool recursive = false);
   bool     exists( std::string name);
   bool     remove( std::string name);
   uint64_t remove_all( std::string name);
   int64_t  file_size( std::string name );
   bool     is_directory( std::string name );
   bool     current_path(std::string path);
   std::vector<std::string> getFileList( std::string path );        
   std::string current_path();                             
   space_info  space( std::string path );                 
   double      getUtilization( std::string path );

   /**
   *  \brief update mod/access times
   *  \param name file on which to operate
   *  \param create whether or not to create if doesn't exist
   *  \return errors from creating or updating time
   **/
   int touch(std::string name, bool create = true);

   /**
   *  \brief return file modification time
   *  \param name filename
   *  \param [out] time pointer to uint64_t for return of usec time of mod
   *  \return 0 on success
   *  \return negative errno values on failure ( 0 - errno)
   **/
   int getLastModTime(std::string name, uint64_t *time);
 
}
}
