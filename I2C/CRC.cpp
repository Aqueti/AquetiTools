/**
 * \file CRC.cpp
 **/

#include <iostream>
#include "CRC.hpp"

namespace atl
{
  /**
  * \brief sets initial value and polynomial
  *
  * \param [in] initial the initial value of the CRC
  * \param [in] poly the polynomial used for encoding
  **/
   CRC16::CRC16(uint16_t initial, uint16_t poly) 
   {
      m_initialValue = initial;
      m_polynomial = poly;

      generateTable();
   }

   /** 
    * \brief generates a lookup table for quick crc calculations
    *
    * \return true on success, false on failure
    **/
   bool CRC16::generateTable() 
   {
      for (int i = 0; i < 256; i++) {
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
   *
   * \param [in] array byte array to operate on
   * \param [in] length number of bytes
   * \return 64-bit calculated CRC 
   *
   * The data in the table constructed by running the algorithmic CRC
   * with initial CRC (crc & 0xFF00), and the input data 0x00. The
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
      CRCMap value;
      value.crc = m_initialValue;

      //Calculate the raw crc
      for(size_t i = 0; i < length; i++) {
        value.crc = ((value.crc << 8) | array[i]) ^ m_crcTable[value.crc >> 8];
      }

      // Augment 16 zero-bits 
      value.crc = ((value.crc << 8) | 0) ^ m_crcTable[value.crc >> 8];
      value.crc = ((value.crc << 8) | 0) ^ m_crcTable[value.crc >> 8];
//    

      //Flip the bytes
      uint8_t first = value.bytes[1];
      value.bytes[1] = value.bytes[0];
      value.bytes[0] = first;

      return value.crc;
   } 
}