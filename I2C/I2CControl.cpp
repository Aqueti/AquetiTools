
#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <CRC.hpp>
#include "I2CControl.hpp"

#include <string.h>

namespace aqt
{
   /**
    * \brief Destructor
    * 
    * On exit, the destructor closes all open files
    **/
   I2CControl::~I2CControl()
   {
      //Close all open file descriptors
      for( auto it = m_filenameToFdMap.begin(); it != m_filenameToFdMap.end(); ++it ) {
        close(it->second);
      }
   }
   
   /**
    * \brief Constructor
    * \param [in] deviceId unique id provided by the application
    * \param [in] filename i2c device file to connect to
    * \param [in] addr i2c device address to connect to
    * \return true on success, false on failure
    *
    * This function establishes an i2c connection for a given device. Each I2C device
    * is acessible via a file interface. If the file specified by filename is already
    * open, the constructor creates a mapping between the deviceId and the file/address
    * pair. If the file is not opened, then it is opened prior to creating the mapping.
    * All file descriptors are closed on exit.
    **/
   bool I2CControl::connect( std::string deviceId, std::string filename, uint16_t addr )
   {
      std::map <std::string,int>::iterator it1;
      std::map <std::string, std::pair<int, uint16_t > >::iterator it2;
   
      //Check if device already connected
      it2 = m_mCamToDeviceMap.find(deviceId);
      if( it2 != m_mCamToDeviceMap.end()) {
         std::cout << "I2C Device "<<deviceId<<" already connected"<<std::endl;
         return false;
      }
   
      //Check if file is already open
      int fd = -1;
      it1 = m_filenameToFdMap.find( filename );
      if( it1 == m_filenameToFdMap.end()) {
         //Open and add to map
         if(( fd = open(filename.c_str(), O_RDWR)) < 0 ) {
            std::cerr << "Unable to open i2 device file "<<filename<<std::endl;
            return false;
         }
   
         //Add to map
         m_filenameToFdMap.insert(std::pair<std::string, int>(filename,fd));
      }
   
      //Set I2C control settings
      if( ioctl(fd, I2C_SLAVE, addr) < 0 ) {
         std::cout << "I2CControl Failed to acquire bus access and/or communicate with "
                   << filename <<":"
                   << std::hex << addr
                   <<std::endl;
         return false;
      }
   
      //Add the device to the map
      m_mCamToDeviceMap[deviceId] = std::make_pair(fd, addr);
   
      return true;
   }
   
   /**
    * \brief reads the specified number of bytes from the bus
    * \param [in] deviceId application provided id for the device
    * \param [in] nbytes number of bytes to read (default = 1)
    * \return vector of received bytes. The vector will have no elements on failure
    *
    * This function reads data from the device specified by the deviceId. This
    * function strips the header and crc bytes from the incoming packets and returns
    * the packet payload.
    **/
   std::vector<uint8_t> I2CControl::readData( std::string deviceId, int nbytes )
   {
      std::vector<uint8_t> message;
   
   
      //Set the offset
      if( !setPointer( m_mCamToDeviceMap[deviceId].first
                     , m_mCamToDeviceMap[deviceId].second 
                     )) 
      {
         std::cout << "Unable to set the pointer for device " << deviceId
                   << " to address "<< std::hex 
                   << m_mCamToDeviceMap[deviceId].second
                   << std::endl;
         return message;
      }

      int fd = m_mCamToDeviceMap[deviceId].first;

      uint8_t buffer[nbytes];
      int count = read( fd, buffer, nbytes);

      int start = -1;
      int end = -1;
      int payload = -1;
      int index = 0;

      //Find the start
      while( index < count ) 
      {
         if( buffer[index] == 0x7e ) {
            start = index;
            payload = start+3;
            index++;
            break;
         }
         index++;
      } 

      if( start < 0 ) { 
         std::cout << "First byte not found!" <<std::endl;
         return message;
      }

      //Find the end
      while( index < count ) 
      {
         if( buffer[index] == 0x7e ) {
            end = index;
            index++;
            break;
         }
         index++;
      }

      if( end == 0 ) { 
         std::cout << "Last byte not found!" <<std::endl;
         return message;
      }

      int bytes = end-payload;

      CRCMap value;
      value.crc = m_crc.calculate( &buffer[payload], bytes );
      
      if(( value.bytes[0] != buffer[start+1])||(value.bytes[1] != buffer[start+2])) {
         printf("CRC Check failed! %02x%02x != %02x%02x \n", value.bytes[0], value.bytes[1], buffer[start+1],buffer[start+2]);

         return message;
      }
      
      for( int i = payload; i < end ; i++ ) {
         message.push_back( buffer[i]);
      }


   
      return message;
   }
   
   /**
    * \brief Genrate an i2C package and send with the given data
    * \param [in] deviceId application provided id for the device
    * \param [in] nbytes number of bytes to read (default = 1)
    *
    * This function writes the payload specified data buffer to the i2c device specified
    * by deviceId. The i2C packet including CRC calculations are handled internally and 
    * should not be included in the buffer to be sent.
    **/
   bool I2CControl::writeData( std::string deviceId, uint8_t * buffer, int nbytes )
   {

      //Calculate the message
      size_t messageSize =nbytes+4;
      uint8_t message[messageSize];
 
      //Cacluate crc
      CRCMap crcMap;
      crcMap.crc = m_crc.calculate( buffer, nbytes);

      message[0] = 0x7e;
      message[1] = crcMap.bytes[0];
      message[2] = crcMap.bytes[1];
      memcpy( &message[3], buffer, nbytes );
      message[messageSize-1] = 0x7e;

 
      //Set the offset
      if( !setPointer( m_mCamToDeviceMap[deviceId].first, m_mCamToDeviceMap[deviceId].second )) {
         std::cout << "Unable to set the pointer for device "<<deviceId<<" to address "<<std::hex << m_mCamToDeviceMap[deviceId].second <<std::endl;
         return false;
      }
      
      int fd = m_mCamToDeviceMap[deviceId].first;
      if( write(fd, message, messageSize) != (int)messageSize ) { 
         std::cout << "Failed to write "<<messageSize<<" bytes to device "<<deviceId<<std::endl;
         return false;
      }
   
      return true;
   }
   
   /** 
    * \brief Function to set the file offset to the correct address
    * \param [in] deviceId component unique id
    * \param [in] address 16bit address to move to
    *
    * This function aligns the file pointer to the appropriate address for 
    * reading and writing. It is called prior to any reads or writes.
    **/
   bool I2CControl::setPointer( int fd, uint16_t address )
   {
      uint8_t * addr = (uint8_t *)(&address);
   
      if( i2c_smbus_write_byte_data( fd, addr[0], addr[1] ) < 0 ) {
         std::cout << "Unable to set pointer to file "<< fd <<std::endl;
         return false;
      }
   
      return true;
   }
   
   /**
    * \brief main testing function
    **/
   bool testI2CControl ( std::string filename, int address )
   {
      std::string device1("test1");
   
      I2CControl i2c;
      if( !i2c.connect( device1, filename, address )) {
         std::cout << "Unable to connect to "<<filename<<" at address "<<address<<std::endl;
         return false;
      }

      return true;
   }
};
