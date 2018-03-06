/**
 * \file TimerTest.cpp
 **/

#include "AquetiToolsTest.h"
#include "StringTools.h"

namespace atl{
   //***********************************************************
   /*!\brief functional test for this class
    *
    * \param [in] printFlag boolean (true prints info to console)
    * \param [in] assertFlag boolean (true stops the program)
    * \return JsonBox::Value containing test results
    */
   //***********************************************************
   JsonBox::Value testStringTools(bool printFlag, bool assertFlag)
   {
       bool rc = true;
       JsonBox::Value resultString; //!< Brief JsonBox value with unit test results
   
       //Test dash case
       std::string input = "ABC-123-XYZ";
       std::string delim = "-";
       std::vector<std::string> result = stringParser( input, delim);
   
       //Make sure we get three appropriate values
       if( result.size() != 3 ) { 
           if(printFlag) {
               std::cout << "StringParser: Incorrect number of returns" << std::endl;
           }
          rc = false;
       }
       else if( result[0].compare("ABC")) {
          if( printFlag ) {
             std::cout << "StringParser: First compare is not valid" <<std::endl;
          }
          rc = false;
       }
       else if( result[1].compare("123")) {
          if( printFlag ) {
             std::cout << "StringParser: Second compare is not valid" <<std::endl;
          }
          rc = false;
       }
       else if( result[2].compare("XYZ")) {
          if( printFlag ) {
             std::cout << "StringParser: Second compare is not valid" <<std::endl;
          }
          rc = false;
       }

       std::cout << "StringTools rc: "<<rc<<std::endl;
       resultString["pass"].setBoolean(rc);

       return resultString;
   }
}
