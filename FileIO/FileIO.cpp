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
      int irc =  mkdir( dirname.c_str(),  S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

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
      if( is_directory( name )) {
         int irc = rmdir( name.c_str());
         if( irc ) {
            std::cerr << "Failed to directory: "<<name<<std::endl;
            return false;
         }
      }
      else {
         if( remove( name.c_str())) {
            std::cerr << "Failed to remove file: "<<name<<std::endl;
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
      DIR * dir;
      struct dirent * ent = NULL;
     
      std::cout << "Remove_all: "<<name<<std::endl;


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
      return count;
   }

  /**
   * \brief determine current path
   * \return Current working path
   **/
   std::string current_path()
   {
      char name[PATH_MAX];
      char * result = getcwd(name, PATH_MAX);

      return std::string( result );
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
         std::cout << "Unable to change directory to "<<newPath<<" with code: "<<irc<<std::endl;
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
}
}
