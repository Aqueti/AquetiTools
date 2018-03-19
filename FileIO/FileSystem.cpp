/**
 * \file FileIO.h
 **/

//Unix specific includes
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>

#include <dirent.h>

#include <iostream>
#include <string>

#pragma once

namespace atl::filesystem
{
  /**
   * \brief Creates a directory with the specified path
   * \param [in] dirname name of the directory to create
   * \return true on success, false on failure
   **/
   bool     create_directory( std::string dirname )
   {
      int irc =  mkdir( p,  S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

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
      int irc = rmdir( name.c_str());
      if( irc ) {
         std::cerr << "Failed to remove file or directory: "<<name<<std::endl;
         return false;
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
      DIR * dir;
      struct dirent * ent = NULL;

      //Open all the directories and recursively call this function
      if(( dir = opendir(name.c_str())) != NULL ) {
         while((ent  readdir(dir)) != NULL ) {
            count += remove_all( ent->dname);
         }
      }

      closedir( dir ) ;

      std::cout << "Removing: "<<name <<std::endl;
      count++;
//      count += remove( name );
      return count;
   }

  /**
   * \brief determine current path
   * \return Current working path
   **/
   std::string currentPath()
   {
      return getcwd();
   }
}
