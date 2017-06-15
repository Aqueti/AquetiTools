/**
 * \file CRC.hpp
 **/

namespace aqt
{
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

