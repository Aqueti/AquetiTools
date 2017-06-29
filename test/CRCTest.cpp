/**
 * \file CRCTest.cpp
 **/

#include "AquetiToolsTest.h"

namespace atl {
/**
* \brief CRC unit test
*
* \return whether or not the test succeeded
**/
JsonBox::Value testCRC()
{
    JsonBox::Value resultString; //!< Brief JsonBox value with unit test results

    uint8_t buffer[256];
    uint16_t expected = 0x03d2;

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

    if (result != expected) { 
       std::cout  << "CRC Check failed. Received "
                  << std::hex << result 
                  << " expected " << std::hex
                  << expected <<std::endl;
      resultString["CRC Check"] = "fail";
      resultString["pass"] = false;
      return resultString;
    }

    resultString["CRC Check"] = "pass";
    resultString["pass"] = true;
    return resultString;
}
}