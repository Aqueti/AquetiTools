//   Copyright Aqueti 2016.

// Distributed under the Boost Software License, Version 1.0.

//    (See accompanying file LICENSE_1_0.txt or copy at

//   http://www.boost.org/LICENSE_1_0.txt)

#include <revision.h>
#include <iostream>
#include <revision.h>
#include "AquetiToolTest.h"
#include <string.h>

using namespace std;


std::vector<std::string> testList{"Timer", "Thread", "MultiThread", "ThreadPool", "LruCache", "TSMap", "TSQueue"};

bool printHelp(void)
{
    cout << "ATLTest" << endl;
    //AquetiTool::printVersion();
    cout << endl;
    cout << "Usage: ./ATLTest <options>" << endl;
    cout << "Options: " << endl;
    cout << "\t-h    prints this help menu and exits" << endl;
    cout << "\t-t    is followed by name of method to be tested, then performs test and exits" << endl;
    cout << "\t-v    indicates that valgrind is being used" << endl;
    cout << "\t--version prints ATL version information\n"<<std::endl;
    cout << "Method names to follow t are: Timer, Thread, "
         << "MultiThread, ThreadPool, TaskManager, TSQueue, BaseBuffer, "
         << "ExtendedBuffer, BaseContainer, TSArray, "
         << "Module BaseContainerArrayMetadata, BaseContainerArray, "
         << "CamImage, HContainer, "
         << "MapContainerMetadata, MapContainer, BaseSocket, SocketServer, "
         << "ContainerSocket, CommandServer, MultiCommandClient, IntKey, "
         << "StringKey, IntPairKey, JsonValidator, TSMap, Interrupt, Property, "
         << "PropertyManager, StreamManager, PropertyTypes, Factory, Messages" << endl;
    cout << endl;
    return 1;
}

/**
 * Main test function
 **/
int main(int argc, char** argv)
{
    bool testAll = true;

    std::cout << "Testing ATL"<<std::endl;

    bool valgrind = false;

    int argCount = 0;
    int i;
    for(i = 1; i < argc; i++) {

        if(!strcmp(argv[i], "-v")) {
            argCount++;
            valgrind = true;
        } else if(!strcmp(argv[i], "-h")) {
            printHelp();
            return 1;
        } else if (!strcmp( argv[i], "--version")) {
            //AquetiTool::printVersion();
            return 1;
        }

        else if(!strcmp(argv[i], "-t")) {
            if(testAll ) {
                testList.empty();
                testAll = false;
            }
            argCount++;
            i++;
            testList.push_back( argv[i]);
        }
    }

    //AquetiTool::printVersion();

    for(std::vector<std::string>::iterator it = testList.begin(); it != testList.end(); ++it ) {
       if( !it->compare("Timer")) {
          if(valgrind) {
              cout << "Timer will not be tested" << endl;
          } else {
             if( !aqt::testTimer() ) {
                cout << "Timer Unit Test Failed!" << endl;
                return 0;
            }
         }
       }
    }

    cout << "All tests completed successfully!" << endl;
    return 0;
}
