/**
 * \file FileIO.h
 **/

//Unix specific includes
#ifdef _WIN32
#include <Windows.h>
#include <direct.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <dirent.h>
#endif

#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <StringTools.h>

#include "FileIO.h"


namespace atl
{
namespace filesystem
{
  /**
   * \brief Creates a directory with the specified path
   * \param [in] dirname name of the directory to create
   * \return true on success, false on failure
   **/
   bool     create_directory( std::string dirname )
   {
#ifdef _WIN32
     int irc = _mkdir(dirname.c_str());
#else
     int irc =  mkdir( dirname.c_str(),  S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

      switch( irc )   
      {
         case 0:
            return true;
         case EACCES:
            std::cerr << "Failed to create directory "<<dirname<<". Permission denied"<<std::endl;
         case ENOENT:
            std::cerr << "Failed to create directory "<<dirname<<". Invalid path"<<std::endl;
         case EROFS:
            std::cerr << "Failed to create directory "<<dirname<<". Read-only file system"<<std::endl;
         default:
            std::cerr << "Failed to create directory "<<dirname<<". Read-only file system"<<std::endl;
      }
      return false;
   }

  /**
   * \brief Function to check if a file exists
   * \param [in] name filename to check if it exists
   * \return true of it exists, false if it doesn't
   **/ 
   bool exists( std::string name )
   {
      struct stat buffer;
      return( stat (name.c_str(), &buffer) == 0 );
   }

  /**
   * \brief Rmoves the specified file/directory
   * \param [in] name name of the directory or file to remove
   * \return true on success, false on failure 
   **/
   bool remove( std::string name)
   {
      //Break name into a array
      std::vector<std::string> pathVect = atl::stringParser(name, "/");
      
      std::string newPath = "";
      for( uint64_t i = 0; i < pathVect.size(); i++ ) {
         newPath.append( pathVect[i]);
         if( !exists(newPath)) {
            std::cerr << newPath <<" does not exist. Unable to remove "<<name<<std::endl;
            return false;
         }
         if( is_directory(newPath)) {
            newPath.append("/");
         }
      }

      if( is_directory( newPath )) {
         int irc = rmdir( name.c_str());
         if( irc ) {
            std::cerr << "Failed to remove directory: "<<newPath<<std::endl;
            return false;
         }
      } 
      else {
         if( ::remove( newPath.c_str())) {
            std::cerr << "Failed to remove file: "<<newPath<<std::endl;
            return false;
         }
      }

      return true;
   }


  /**
   * \brief Removes the directory and all of its contents
   * \param [in] name name of the directory to delete
   * \return Number of items deleted. 0 indicates nothing deleted.
   *
   * This function recursively removes all subdirectories
   **/
   uint64_t remove_all( std::string name) 
   {
     uint64_t count = 0;
#ifdef _WIN32
     std::cerr << "remove_all() not yet implemented on Windows" << std::endl;
#else
      DIR * dir;
      struct dirent * ent = NULL;
     
      //Recursively call this function on all directories except the current
      //and it's parent
      if(( dir = opendir(name.c_str())) != NULL ) {
         while((ent=readdir(dir)) != NULL ) {
            std::string item= ent->d_name;
            if((( item.compare("."))&&(item.compare(".."))))
            {
               std::string itemName = name;
               itemName.append("/");
               itemName.append(item);

               if( is_directory( itemName.c_str())) {
                  count += remove_all( itemName.c_str());
               } 
               else {  
                  ::remove( itemName.c_str());
               }
            }
         }
      }

      closedir( dir ) ;

      count += remove( name );
#endif
      return count;
   }

  /**
   * \brief determine current path
   * \return Current working path
   **/
   std::string current_path()
   {
#ifdef _WIN32
     // Get the length of the string, then allocate and read it.
     DWORD len = GetCurrentDirectory(0, nullptr);
     std::vector<char> result(len);
     if (len != GetCurrentDirectory(len, result.data())) {
       std::cerr << "Unable to get current directory" << std::endl;
       return "";
     }
     return result.data();
#else
      char name[PATH_MAX];
      char * result = getcwd(name, PATH_MAX);

      return std::string( result );
#endif
   }

  /**
   * \brief Changes the current working path
   * \return true on success, false on failure
   **/
   bool current_path( std::string newPath)
   {
      bool rc = true;

      int irc = chdir( newPath.c_str());
      if( irc != 0 ) 
      {
         std::cerr << "Unable to change directory to "<<newPath<<" with code: "<<irc<<std::endl;
         rc = false;
      }

      return rc;
   }

  /**
   * \brief Checks if a file is a directorys
   * \param [in] name name of file to check
   * \return true if the the file is a directory, false otherwise
   **/
   bool is_directory( std::string name )
   {
      struct stat s;

      if( stat( name.c_str(), &s) == 0 ) {
         if( s.st_mode & S_IFDIR ) {
            return true;
         }
      }

      return false;
   }

  /**
   * \brief gets disk information for the current directory
   * \param [in] path directory to check
   **/
   space_info space( std::string path )
   {
     space_info si;
#ifdef _WIN32
     ULARGE_INTEGER available, capacity, free;
     if (GetDiskFreeSpaceEx(path.c_str(), &available, &capacity, &free)) {
       si.capacity = capacity.QuadPart;
       si.free = free.QuadPart;
       si.available = available.QuadPart;
     };
#else
      struct statvfs stat;

      int rc =  statvfs( path.c_str(), &stat );
      if( rc  == 0 ) {
         si.capacity  = stat.f_bsize * stat.f_blocks;
         si.free      = stat.f_bsize * stat.f_bfree;
         si.available = si.capacity - si.free;
      }
#endif
      return si;
   }

  /**
   * \brief lists all files and directories in the given path
   **/
   std::vector<std::string>getFileList( std::string path )
   {
      std::vector<std::string> fileList;
      DIR *dir;
      struct dirent *ent;
      if(( dir = opendir( path.c_str() != NULL ) {
         while(( ent = readdir( dir)) != NULL ) {
            fileList.push_back( ent->d_name );
         }
         closedir(dir);
      }
      else {
         perror("");
      }

      return fileList;
   }
}
}
