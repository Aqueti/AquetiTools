/**
 * \file I2CControl.hpp
 **/

#pragma once

#ifndef _WIN32
#include <linux/i2c-dev.h>
#endif
#include <string>
#include <vector>
#include <map>

#include <stdint.h>
#include <CRC.hpp>

namespace atl
{

   /**
   * \brief class to establish i2c connection with a device
   **/
   class I2CControl
   {
      public:
         bool connect(std::string deviceId,  std::string filename, uint16_t addr);
         ~I2CControl();
         std::vector<uint8_t> readData(std::string deviceId, int nbytes = 1);
         bool writeData(std::string deviceId, uint8_t * buffer, int nbytes);
    
      private:
         bool m_crcCheckFlag = true;             //!< Flag to indicate we need to validate CRC
         CRC16 m_crc;                            //!< CRC class
         bool setPointer(int fd, uint16_t address);
         std::map <std::string, int > m_filenameToFdMap;                //!< map of open file pointers by name
         std::map <std::string, std::pair<int, uint16_t > > m_mCamToDeviceMap; //!< map of devices to file/address pairs
   };
}