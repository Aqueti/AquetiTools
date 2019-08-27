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
#include <fcntl.h>
#include <sys/types.h>
#endif

#include <errno.h>
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
bool _create_directory( std::string dirname );
  /**
   * \brief Creates a directory with the specified path
   * \param [in] dirname name of the directory to create
   * \return true on success, false on failure
   **/
   bool create_directory( std::string dirname, bool recursive)
   {
	   if(!recursive) return _create_directory(dirname);

#ifdef fleabug
std::cout << "creating directory " << dirname << std::endl;
#endif
	   std::vector<std::string> pathTokens = stringParser(dirname, "/");
	   std::string path;
	   for(int i =0; i< pathTokens.size(); ++i){

		   // handle repeated, leading, trailing delimiters
		   if(pathTokens[i].empty()) { 
			   continue;
		   } else {
			   if(i){
				  path.append("/"); 
			   }
		   }

		   path.append(pathTokens[i]);

#ifdef fleabug
std::cout << "token is: " << pathTokens[i] << std::endl;
std::cout << "checking path " << path << std::endl;
#endif
		
		   if(!exists(path)){
#ifdef fleabug
std::cout << path << " doesn't exist, creating directory" << std::endl;
#endif
			   bool rc = _create_directory(path);
			   if(!rc){
				   return false;
			   }
		   } else {
			   if(!is_directory(path)){
#ifdef fleabug
std::cout << "path exists, but isn't directory" << std::endl;
#endif
					return false;
				}
			}
	   }
	   return true;
   }



bool _create_directory( std::string dirname )
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
         case EEXIST:
            std::cerr << "Failed to create directory "<<dirname<<". Pathname already exists" << std::endl;
         case ENOENT:
            std::cerr << "Failed to create directory "<<dirname<<". Invalid path"<<std::endl;
         case EROFS:
            std::cerr << "Failed to create directory "<<dirname<<". Read-only file system"<<std::endl;
         default:
            std::cerr << "Failed to create directory "<<dirname<<". " <<std::endl;
            perror("Error info: ");
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
      //std::vector<std::string> pathVect = atl::stringParser(name, "/");
      //
      //std::string newPath = "";
      //for( uint64_t i = 0; i < pathVect.size(); i++ ) {
      //   if( pathVect[i]=="" )  {
      //      newPath.append("/");
      //   } else {
      //      newPath.append( pathVect[i]);
      //   }
      //   if( !exists(newPath)) {
      //      std::cerr << newPath <<" does not exist. Unable to remove "<<name<<std::endl;
      //      return false;
      //   }
      //   if( is_directory(newPath)) {
      //      newPath.append("/");
      //   }
      //}

      if( is_directory( name )) {
         int irc = rmdir( name.c_str());
         if( irc ) {
            //std::cerr << "Failed to remove directory: "<<name<<std::endl;
            return false;
         }
      } 
      else {
         if( ::remove(name.c_str()) != 0) {
            //std::cerr << "Failed to remove file: "<<name<<std::endl;
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
         si.capacity  = stat.f_frsize * stat.f_blocks;
         si.free      = stat.f_bsize * stat.f_bavail;
         si.available = si.capacity - si.free;
      }
#endif
      return si;
   }

  /**
   * \brief lists all files and directories in the given path
   * \param[in] path the target directory
   * \param[in] all if true, adds . and .. to the return list
   **/
   std::vector<std::string>getFileList(std::string path, bool all)
   {
     std::vector<std::string> fileList;
#ifdef _WIN32
     std::cerr << "getFileList() not yet implemented on Windows" << std::endl;
#else
      DIR *dir = NULL;
      struct dirent *ent;
      dir = opendir( path.c_str());
      if( dir != NULL ) {
         while(( ent = readdir( dir)) != NULL ) {
             std::string fname(ent->d_name);
             if (all || fname.find(".") == std::string::npos) {
                fileList.push_back( ent->d_name );
             }
         }
         closedir(dir);
      }
      else {
         perror("");
      }
#endif
      return fileList;
   }

  /**
   * \brief gets the size of the specified file
   * \return returns the file size in bytes, -1 on error
   **/
   int64_t file_size( std::string filename )
   {
      //Check if the file exists
      if( !exists(filename)) {
         return -1;
      }

      struct stat stat_buf;
      int rc = stat(filename.c_str(), &stat_buf );
      return rc == 0 ? stat_buf.st_size : -1;
      
      
   }

  /**
   * \brief Shows disk utilization for specified drive
   **/
   double getUtilization( std::string path )
   {
      double util = 0;
      //Calculate the max utilization for each drive
      filesystem::space_info si = filesystem::space( path );
      if( si.capacity != 0 ) {
         util = 1.0-(double)si.free / (double)si.capacity;
      }

      return util;

   }

int touch(std::string name, bool create)
{
#ifdef _WIN32
     std::cerr << "touch not yet implemented on Windows" << std::endl;
	 return -1;
#else
 
	// open

	 int fd = open(name.c_str(), O_CREAT | O_WRONLY | O_NONBLOCK | O_NOCTTY, 0666);
	 if(fd < 0) {
		 int err = errno;
		 std::cerr << __FUNCTION__ << " error opening file" << std::endl;
		 // TODO: use api returns 
		 return err;
	 }

	 struct timespec times[2];
	 times[0].tv_nsec = UTIME_NOW;
	 times[1].tv_nsec = UTIME_NOW;

	 int rv = futimens(fd,times);
	 if(rv < 0) {
		 int err = errno;
		 std::cerr << __FUNCTION__ << " error setting file access/mod time" << std::endl;
		 // TODO: use api returns 
		 return err;
	 }

	 close(fd);

	 return 0;

#endif
}

int getLastModTime(std::string name, uint64_t *time)
{
#ifdef _WIN32
     std::cerr << "getLastModTime not yet implemented on Windows" << std::endl;
	 return -ENOSYS;
#else
 
	struct stat statBuf;
	int rv = stat(name.c_str(), &statBuf);
	if(rv < 0){ 
		int err = errno;
		perror(0);
		return 0 - err;
	}
	if(time){
		*time = statBuf.st_mtim.tv_sec * 1e6 + statBuf.st_mtim.tv_nsec / 1e3;
	} else {
		return -EINVAL;
	}
	return 0;
#endif
}

}
}
