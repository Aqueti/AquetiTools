/**
 * \file TimerTest.cpp
 **/
#include <JsonBox.h>
#include "AquetiToolsTest.h"

namespace atl
{
JsonBox::Value testFileIO( bool printFlag, bool assertFlag  )
{
   std::cout << "Creatig test"<<std::endl;
   atl::filesystem::create_directory("test");
   atl::filesystem::create_directory("test/test2");

   if( !atl::filesystem::remove("test")) {
      std::cout << "Failed to remove directory (expected)"<<std::endl;
   }

   atl::filesystem::remove_all("test");

   JsonBox::Value result;
   result["FileIO"]["pass"].setBoolean(true);
   return result;
}
}
