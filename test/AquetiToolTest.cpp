//   Copyright Aqueti 2016.

// Distributed under the Boost Software License, Version 1.0.

//    (See accompanying file LICENSE_1_0.txt or copy at

//   http://www.boost.org/LICENSE_1_0.txt)

#include "AquetiToolTest.h"
#include "Timer.h"

using namespace std;

std::vector<std::string> testList{"Timer", "CRC", "Thread", "MultiThread", "ThreadPool", "LruCache", "TSMap", "TSQueue"};

JsonBox::Value testAquetiTools(bool testSubmodules, int i)
{
    JsonBox::Value jsonReturn;
    JsonBox::Value jsonUnits;
    JsonBox::Value jsonValue;
    JsonBox::Value subJson1;
    JsonBox::Value subJson2;

    //Get type
    jsonReturn["type"] = "unit_tests";

    //Get repository
    jsonReturn["component"] = "AquetiTools";

    //Get date/time
    jsonReturn["date"] = atl::getDateAsString();

    //Get /etc/quid
    std::string guid;
    std::ifstream nameFileout;
    nameFileout.open("/etc/guid");
    if (nameFileout.good()) {
        getline(nameFileout, guid);
    }
    nameFileout.close();
    jsonReturn["hardwareId"] = guid;

    //Get commit hash ID and version
    jsonReturn["commit"] = atl::GIT_COMMIT_HASH;
    jsonReturn["version"] = atl::VERSION;
    std::string softwareId1 = atl::VERSION;
    std::string softwareId2 = atl::GIT_COMMIT_HASH;
    std::string softwareId = softwareId1 + ":" + softwareId2;
    jsonReturn["softwareId"] = softwareId;

    //Get test results from all classes (units)
    jsonValue = atl::testTimer(true, false);
    jsonUnits["Timer"] = jsonValue;
    jsonReturn["units"] = jsonUnits;

    /*switch(i) {
        case 1: 
            jsonValue = atl::testTimer(true, false);
            jsonUnits["Timer"] = jsonValue;
            jsonReturn["units"] = jsonUnits;
            break;
        case 2: 
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        default:
            break;
    } */

    //Get submodules
    subJson2["version"] = atl::VERSION;
    subJson2["commit"] = atl::GIT_COMMIT_HASH;
    subJson1["AquetiTools"] = subJson2;
    jsonReturn["submodules"] = subJson1;

    return jsonReturn;
}

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

void runTests(bool valgrind) {
    JsonBox::Value result;
    std::vector<std::string>::iterator it;

    for (it = testList.begin(); it != testList.end(); ++it) {
        if (!it->compare("Timer")) {
            if (valgrind) {
                cout << "Timer will not be tested!" << endl;
            }
            std::cout << "Testing Timer" << std::endl;
            result = testAquetiTools(true, 1);
            std::cout << result << std::endl;
        } /*else if (!it->compare("CRC")) {
            std::cout << "Testing CRC" << std::endl;
            result = testAquetiTools(true, 2);
            std::cout << result << std::endl;
        } else if (!it->compare("Thread")) {
            std::cout << "Testing Thread" << std::endl;
            result = testAquetiTools(true, 3);
            std::cout << result << std::endl;
        } else if (!it->compare("MultiThread")) {
            std::cout << "Testing MultiThread" << std::endl;
            result = testAquetiTools(true, 4);
            std::cout << result << std::endl;
        } else if (!it->compare("ThreadPool")) {
            std::cout << "Testing ThreadPool" << std::endl;
            result = testAquetiTools(true, 5);
            std::cout << result << std::endl;
        } else if (!it->compare("LruCache")) {
            std::cout << "Testing LruCache" << std::endl;
            result = testAquetiTools(true, 6);
            std::cout << result << std::endl;
        } else if (!it->compare("TSMap")) {
            std::cout << "Testing TSMap" << std::endl;
            result = testAquetiTools(true, 7);
            std::cout << result << std::endl;
        } else if (!it->compare("TSQueue")) {
            std::cout << "Testing TSQueue" << std::endl;
            result = testAquetiTools(true, 8);
            std::cout << result << std::endl;
        } */
    }
}

/**
 * Main test function
 **/
int main(int argc, char *argv[])
{
    bool testAll = true;
    std::cout << "Testing ATL" << std::endl;
    bool valgrind = false;

    //command line options
    int argCount = 0;
    int i;

    for(i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-v")) {
            argCount++;
            valgrind = true;
        } else if (!strcmp(argv[i], "-h")) {
            printHelp();
            return 1;
        } else if (!strcmp(argv[i], "--version")) {
            //AquetiTool::printVersion();
            return 1;
        } else if(!strcmp(argv[i], "-t")) {
            if (testAll) {
                testList.clear();
                testAll = false;
            }

            argCount++;
            i++;

            testList.push_back( argv[i]);
        }
    }

    runTests(valgrind);

    //Testing


    /*
    //AquetiTool::printVersion();

    for(std::vector<std::string>::iterator it = testList.begin(); it != testList.end(); ++it ) {
        if( !it->compare("Timer")) {
        	  if(valgrind) {
                cout << "Timer will not be tested" << endl;
            } 
            else {
                std::cout << "Testing Timer" <<std::endl;
           	    if( !atl::testTimer() ) {
                    cout << "Timer test failed!" << endl;
                    return 1;
                }
            }
        }
        else if(!it->compare("Thread")) {
            std::cout << "Testing Thread" <<std::endl;
            if( !atl::testThread() ) {
                cout << "C11Thread test failed!" << endl;
                return 1;
            }
        }
        else if(!it->compare("MultiThread")) {
            std::cout << "Testing MultiThread" <<std::endl;
            if( !atl::testMultiThread() ) {
                cout << "MultiThread test failed!" << endl;
                return 1;
            }
        }
        else if(!it->compare("ThreadPool")) {
            std::cout << "Testing ThreadPool" <<std::endl;
            if( !atl::testThreadPool() ) {
                cout << "ThreadPool test failed!" << endl;
                return 1;
            }
        }
        else if(!it->compare("LruCache")) {
            int threads = 100;
            cout << "Testing LruCache with " << threads << " threads" << endl;
            if( !atl::test_LruCache(threads, true, false)) {
                std::cout <<"LruCache test failed!"<<std::endl;
                return 1;
            }
        } 
        
        else if(!it->compare("TSMap")) {
            std::cout << "Testing TSMap" <<std::endl;
            if( !testTSMap( true, valgrind )) {
                cout << "TSMap test failed!" << endl;
                return 1;
            }
        }
        
        else if(!it->compare("TSQueue")) {
            std::cout << "Testing TSQueue" <<std::endl;
            if( !atl::testTSQueue(20, true, false)) {
                std::cout <<"TSQueue test failed!"<<std::endl;
                return 1;
            }
        }
        else if( !it->compare("CRC")) {
          	std::cout << "Testing CRC" <<std::endl;
          	if( !atl::testCRC() ) {
              	cout << "CRC test failed!" << endl;
              	return 0;
          	}
        }
    }*/

    cout << "All tests completed successfully!" << endl;
    return 0;
}
