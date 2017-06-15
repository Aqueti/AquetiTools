/**
 * \file CRC.hpp
 **/
#ifndef CRC_HPP
#define  CRC_HPP

#include <map>

namespace aqt
{

   std::map<char, int > charToHexMap = {
      {'0', 0x0},
      {'1', 0x1},
      {'2', 0x2},
      {'3', 0x3},
      {'4', 0x4},
      {'5', 0x5},
      {'6', 0x6},
      {'7', 0x7},
      {'8', 0x8},
      {'9', 0x9},
      {'A', 0xa},
      {'B', 0xb},
      {'C', 0xc},
      {'D', 0xd},
      {'E', 0xe},
      {'F', 0xf}
   };

   /**
    * \brief Union to make crc easily accessible as uint16 or as bytes
    **/
   union CRCMap
   {
      uint16_t crc;
      uint8_t bytes[2];
   };


   /**
    * \brief class for calculating 16 bit CRC codes
    **/
   class CRC16
   {
      public:
         CRC16( uint16_t intial = 0x0000, uint16_t poly = 0x1021 );
         uint16_t calculate( uint8_t * array, size_t length );
      
      private:
         uint16_t m_crcTable[256];
         uint16_t m_initialValue = 0x0000;  //!< Starting value for the CRC
         uint16_t m_polynomial = 0x1021;   //!< Polynomial used to caclculate the CRC

         bool     generateTable();
   };

   bool testCRC();
}
#endif
