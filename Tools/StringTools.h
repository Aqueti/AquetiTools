#include <string>
#include <vector>

namespace atl
{
  /**
   * \brief Function that parses a string based on a delimiter
   **/ 
   std::vector<std::string> stringParser( std::string input, std::string delim );

 /**
  * \brief Function that converts an integer to a string of given precision
  **/
  std::string intToString( uint64_t value, int width );
}
