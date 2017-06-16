#include <iostream>
#include "I2CControl.hpp"
#include <string.h>
//#include <revision.h>
#include <CRC.hpp>

void printHelp()
{
   std::cout << "I2C Controller"<<std::endl;
 //  std::cout << "AQT VERSION: "<<aqt::VERSION<<", Commit: "<<aqt::GIT_COMMIT_HASH<<", Date: "<<aqt::BUILD_DATE<<std::endl;
   std::cout << "Options:"<<std::endl;
   std::cout << "\t-a device address"<<std::endl;
   std::cout << "\t-c command to send (start with 0x to make hex)"<<std::endl;
   std::cout << "\t-h print his help menu"<<std::endl;
   std::cout << "\t-f filename of the i2c device"<<std::endl;
}

/**
 * \brief main function
 **/
int main( int argc, char * argv[])
{
   uint8_t buffer[1024];

   //Default message
   size_t  byteCount = 5;
   buffer[0] = 0x10;
   buffer[1] = 1;
   buffer[2] = 1;
   buffer[3] = 6;
   buffer[4] = 0;
//   buffer[4] = 0x10;

   std::string device1("device");
   std::string filename=("/dev/i2c-7");
   uint16_t address = 0x60;

   //Parse arguments
   int argCount = 0;                         //Track number of arguments
   for( int i = 1; i < argc; i++ ) {
        if( !strcmp(argv[i], "-h" )) {
            printHelp();
            exit(1);
        } else if( !strcmp(argv[i], "-a" )) {
            argCount++;
            i++;
            if( i > argc ) {
                std::cout << "-a must provide and address" << std::endl;
                exit(1);
            }

            if( argv[i][1] != 'x' ) {
                std::cout << "-a address must be in the form: 0x60 where 60 is the addres (hex)" << std::endl;
                exit(1);
            }

            sscanf( argv[i], "%hx", &address);
            
        } else if( !strcmp(argv[i], "-f" )) {
            argCount++;
            i++;
            if( i > argc ) {
                std::cout << "-f must provide a filename " << std::endl;
                exit(1);
            }
            filename = std::string( argv[i]);
        } else if( !strcmp(argv[i], "-c" )) {
            argCount++;
            i++;
            if( i > argc ) {
                std::cout << "-c option must specify a command" << std::endl;
                exit(1);
            }

            //If the second character is an x, we have hex
            size_t arglen = strlen(argv[i]);

            //We are an integer, 
            if((arglen < 2 )||( argv[i][1] != 'x')) {
               for( size_t j = 0; j < arglen; j++ ) {
                  buffer[j] = argv[i][j] - '0';
                  printf("Int: %c => %02x\n", argv[i][j], argv[i][j]-'0');
               }
               byteCount = arglen;
            }
            //We are hex
            else {
               for( size_t j = 2; j < arglen; j++ ) {
                  if(( argv[i][j] >= '0' )&&(argv[i][j] <= '9'))
                     buffer[j-2] = argv[i][j] - '0';
                  else if(( argv[i][j] >= 'A' )&&(argv[i][j] <= 'F')) 
                     buffer[j-2] = argv[i][j] - 'A'+ 10;
                  else if(( argv[i][j] >= 'a' )&&(argv[i][j] <= 'f')) 
                     buffer[j-2] = argv[i][j] - 'a'+ 10;
                  else {
                     std::cout << "Invalid command: "<<argv[i][j] << std::endl;
                     printHelp();
                     exit(1);
                  }
               }
               byteCount = arglen-2;
            }
        } else {
            std::cout << "unknown command "<<argv[i]<< std::endl;
            printHelp();
            exit(1);
        }
   }

   std::cout << "Filename: "<<filename<<std::endl;
   printf("Device Address: 0x%hx\n", address );
   //Connect to the i2c device
   aqt::I2CControl i2c;

   bool rc= i2c.connect( device1, filename, address );

   if( !rc ) {
      std::cout << "Unable to connect to "<<filename<<" at address "<<address<<std::endl;
      return false;
   }

/* raw package
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
*/

   printf("Message Sent:\n");
   printf("   hex: ");
   for( size_t i =0; i < byteCount; i++ ) {
      printf("%02x ", buffer[i]);
   }
   printf("\n   dec: ");
   for( size_t i =0; i < byteCount; i++ ) {
      printf("%d ", buffer[i]);
   }

   printf("\n");

   //Write the buffer
   if( !i2c.writeData( device1, buffer, byteCount )) {
      printf("Failed to write data to device:\n");
      for( size_t i = 0; i < byteCount; i++ ) {
         printf("%02x", buffer[i]);
      }

      exit(1);
   }

   std::vector<uint8_t> message = i2c.readData( device1, 128 );

   if( message.size() == 0 ) { 
      std::cout <<" CRC check failed!"<<std::endl;
   }
   else {
      printf("Reponse: \n");

      printf("   hex: ");
      for( size_t i =0; i < message.size(); i++ ) {
         printf("%02x ", message[i]);
      }
      printf("\n   dec: ");
      for( size_t i =0; i < message.size(); i++ ) {
         printf("%d ", message[i]);
      }

   }

   std::cout << std::endl;
}
