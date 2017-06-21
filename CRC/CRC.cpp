/**
 * \file CRC.cpp
 * 
 **/
#include <iostream>
#include "CRC.hpp"

namespace atl
{

   CRC16::CRC16(uint16_t initial, uint16_t poly) 
   {
      m_initialValue = initial;
      m_polynomial = poly;

      generateTable();
   }

   /** 
    * \brief generates a lookup table for quick crc calculations
    * \return true on success, false on failure
    **/
   bool CRC16::generateTable() 
   {
      std::cout << "Generating table with "<< std::hex << m_polynomial<<std::endl;
      for ( int i = 0; i < 256; i++) {
         uint16_t result = i << 8;
         for (int j = 0; j < 8; j++) {
            // Flag for XOR if leftmost bit is set 
            bool xor_flag = result & 0x8000;

            // Shift CRC
            result <<= 1;

            // Perform the XOR 
            if (xor_flag) {
               result ^= m_polynomial;
            }
         }

         m_crcTable[i] = result;
      }

      return true;
  }

  /**
   * \brief Function to generate a crc code
   * \param [in] array byte array to operate on
   * \param [in] length number of bytes
   *
   * The data in the table constructed by running the algorithmic CRC
   * with inital CRC (crc & 0xFF00), and the input data 0x00. The
   * resulting CRC can be found in the table at (crc >> 8).
   *
   * Since the operation is performed on a zero rightmost half of the
   * CRC, and a zero input byte, we can construct the new CRC by usin
   * the rightmost half of the current CRC as the new leftmost half, and
   * the byte as the leftmost half. We then XOR this construction with
   * the contents of the table at position (crc >> 8). 
   **/
   uint16_t CRC16::calculate(uint8_t * array, size_t length) 
   {
      uint16_t crc = m_initialValue;
      std::cout  << "Calculating with initial value "<<std::hex<<m_initialValue<<std::endl;

      //Caclulate the raw crc
      for(size_t i = 0; i < length; i++) {
        crc = ((crc << 8) | array[i]) ^ m_crcTable[crc >> 8];
      }

      std::cout << "PreFinal: "<<std::hex<<crc<<std::endl;
      // Augment 16 zero-bits 
//      for ( size_t i = 0; i < 2; i++) {
        crc = ((crc << 8) | 0 ) ^ m_crcTable[crc >> 8];
        crc = ((crc << 8) | 0 ) ^ m_crcTable[crc >> 8];
//     }

      std::cout << "Post Final: "<<std::hex<<crc<<std::endl;
      return crc;
   } 


   /**
    * \brief CRC unit test
    **/
   bool testCRC()
   {
      uint8_t buffer[256];
      uint16_t expected = 0xd203;

      //Check evetar exampel
      size_t datalen = 10;
      buffer[0] = 0x10;
      buffer[1] = 0x01;
      buffer[2] = 0x01;
      buffer[3] = 0x12;
      buffer[4] = 0x05;
      buffer[5] = 0x00;
      buffer[6] = 0x07;
      buffer[7] = 0x08;
      buffer[8] = 0x08;
      buffer[9] = 0xf6;

      CRC16 crc;

      uint16_t result = crc.calculate( buffer, datalen);

      if( result != expected ) { 
         std::cout  << "CRC Check failed. Received "
                    << std::hex << result 
                    << " expected " << std::hex
                    << expected <<std::endl;
      }

        return true;
    }
}



