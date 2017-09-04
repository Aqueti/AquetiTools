/**
 * \file testAquetiTools.cpp
 **/

#include "AquetiToolsTest.h"

std::vector<std::string> unitList{"Timer", "CRC", "Thread", "MultiThread", "ThreadPool", "LruCache", "TSMap", "TSQueue", "TaskManager"}; //!< List of units that tests must be run on 

/**
 * \brief prints out help to user
 **/
bool printHelp(void)
{
    std::cout << "testAquetiTools" << std::endl;
    //AquetiTool::printVersion();
    std::cout << std::endl;
    std::cout << "Usage: ./testAquetiTools <options>" << std::endl;
    std::cout << "Options: " << std::endl;
    std::cout << "\t-h    prints this help menu and exits" << std::endl;
    std::cout << "\t-t    is followed by name of method to be tested, then performs test and exits" << std::endl;
    std::cout << "\t-v    indicates that valgrind is being used" << std::endl;
    std::cout << "\t-n    indicates that output should not be inserted into database" << std::endl;
    std::cout << "\t-s    indicates submodules are not to be tested" << std::endl;
    //std::cout << "\t--version prints ATL version information\n"<< std::endl;
    std::cout << "Method names to follow t are: Timer, Thread, "
         << "MultiThread, ThreadPool, TaskManager, TSQueue, "
         << "TSMap, " << std::endl;
    std::cout << std::endl;
    return 1;
}

/**
 * \brief main function
 **/
int main(int argc, char *argv[])
{
    bool testSubmodules = true;
    bool testAll = true;
    bool valgrind = false;
    bool insert = true;
    bool printFlag = true;
    bool assertFlag = false;
    //bool insert = true;

    //command line options
    int i;
    for(i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-v")) {
            valgrind = true;
        } else if(!strcmp(argv[i], "-t")) {
            if (testAll) {
                unitList.clear();
                testAll = false;
            }
            i++;
            unitList.push_back( argv[i]);
        } else if(strcmp(argv[i], "-n") == 0){
            insert = false;
        } else if(strcmp(argv[i], "-s") == 0){
            testSubmodules = false;
        } else if(strcmp(argv[i], "-h") == 0){
            printHelp();
            return 0;
        } else {
            std::cout << "Bad option used..." << std::endl;
            std::cout << "Exiting!" << std::endl;
            return 1;
        }
    }

    //run tests
    std::cout << "Testing AquetiTools..." << std::endl;
    JsonBox::Value result = atl::testAquetiTools(testSubmodules, printFlag, assertFlag, valgrind, unitList);
    if(result["pass"] == true){
        std::cout << "AquetiTools passed successfully!" << std::endl;
        return 0;
    }
    else{
        std::cout << "AquetiTools failed to pass!" << std::endl;
        return 1;
    }


    //connect to database and insert JsonValue if "-n" was not used
    if(insert){
#ifdef USE_MONGO
        std::cout << "Inserting unit test results in database..." << std::endl;
        mongoapi::MongoInterface mi;
        bool connected = mi.connect("aqueti");
        if(connected){
            std::string id = mi.insertUnitTests("unit_tests", result);
            if(id != "0"){
                std::cout << "Results inserted in database successfully!" << std::endl;
            }
        }
        else{
            std::cout << "Failed to insert unit test results!" << std::endl;
            return 0;
        }
#else
        std::cout << "Failed to insert unit test results - Mongo not found" << std::endl;
        return 0;
#endif //USE_MONGO
    }

    return 0;
}
