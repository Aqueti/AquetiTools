/**
 * \file CRC.hpp
 **/
#ifndef CRC_HPP
#define  CRC_HPP

#include <map>
#include "JsonBox.h"

namespace atl
{
   /**
    * \brief Union to make crc easily accessible as uint16 or as bytes
    **/
   union CRCMap
   {
      uint16_t crc;                       //!< Unsigned 16-bit CRC           
      uint8_t bytes[2];                   //!< Bytes that make up crc
   };


   /**
    * \brief class for calculating 16 bit CRC codes
    **/
   class CRC16
   {
      public:
         CRC16(uint16_t initial = 0x0000, uint16_t poly = 0x1021);
         uint16_t calculate(uint8_t * array, size_t length);
      
      private:
         uint16_t m_crcTable[256];          //!< Calculation table of CRC codes
         uint16_t m_initialValue = 0x0000;  //!< Starting value for the CRC
         uint16_t m_polynomial = 0x1021;   //!< Polynomial used to calculate the CRC

         bool     generateTable();
   };
}
#endif