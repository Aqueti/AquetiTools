#include "StringTools.h"
#include <sstream>
#include <iomanip>

namespace atl
{
  /**
   * \brief Function that parses a string based on a delimiter
   **/ 
   std::vector<std::string> stringParser( std::string input, std::string delim )
   {
      std::vector<std::string> resultVector;
      auto start = 0U;
      auto end = input.find(delim);

      //Loop until the end
      while( end != std::string::npos )
      {
         resultVector.push_back( input.substr(start, end-start)); 
         start = end+delim.length();
         end   = input.find( delim, start );
      }

      resultVector.push_back( input.substr(start, end-start)); 

      return resultVector;
   }


 /**
  * \brief Function that converts an integer to a string of given precision
  **/
  std::string intToString( uint64_t value, int width = 16 ) 
  {
     std::stringstream ss;
     ss << std::setfill('0') << std::setw(width) << value;

     return ss.str();
  }

}
