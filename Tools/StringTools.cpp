#include "StringTools.h"

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
}
