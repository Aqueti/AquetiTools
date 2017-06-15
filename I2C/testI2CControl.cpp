
#include <iostream>
#include "I2CControl.hpp"

int main( void )
{
   std::cout << "Testing I2C Control"<<std::endl;
   std::string device1("test1");

   std::string filename=("/dev/i2c-7");

   uint16_t address = 0x60;

   I2CControl i2c;
   std::cout << "Connecting device "<<device1<<" at file location "<<filename<<" and address 0x"<<std::hex << address <<std::endl; 

   bool rc= i2c.connect( device1, filename, address );

   if( !rc ) {
      std::cout << "Unable to connect to "<<filename<<" at address "<<address<<std::endl;
      return false;
   }

   std::cout << "Connected to device "<<device1 << " at location "<< filename << std::endl;


   uint8_t buffer[10];
   buffer[0] = 126;
   buffer[1] = 239;
   buffer[2] = 120;
   buffer[3] = 16;
   buffer[4] = 1;
   buffer[5] = 1;
   buffer[6] = 6;
   buffer[7] = 0;
   buffer[8] = 126;

   //Write the buffer
   i2c.writeData( device1, buffer, 9 );

   std::vector<uint8_t> message = i2c.readData( device1, 128 );

   std::cout << "Return: ";
   for( int i = 0; i < message.size(); i++ ) {
      printf("%02x ", message[i]);
   }
   std::cout << std::endl;

   std::cout << "\nReturn Decimal: ";
   for( int i = 1; i < message.size(); i++ ) {
      printf("%d ", message[i]);
   }
   std::cout << std::endl;
}
