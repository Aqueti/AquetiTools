
#include "AquetiToolsTest.h"

using namespace std;
std::vector<std::string> unitList{"Timer", "CRC", "Thread", "MultiThread", "ThreadPool", "LruCache", "TSMap", "TSQueue"};

int main(int argc, char *argv[])
{
    bool testSubmodules = true;
    bool testAll = true;
    bool valgrind = false;
    bool insert = true;

    //command line options
    int i;
    for(i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-v")) {
            valgrind = true;
        } else if(!strcmp(argv[i], "-u")) {
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
        }
    }

    //run tests
    std::cout << "Testing AquetiTools..." << std::endl;
    JsonBox::Value result = testAquetiTools(unitList, testSubmodules, valgrind);
    std::cout << result << std::endl;
    if(result["pass"] == true){
        std::cout << "AquetiTools passed successfully!" << std::endl;
    }
    else{
        std::cout << "AquetiTools failed to pass!" << std::endl;
    }

/*
    //connect to database and insert JsonValue if "-n" was not used
    std::cout << "Inserting unit test results in database..."
    if(insert){
        mongoapi::MongoInterface mi;
        bool connected = mi.connect("aqueti");
        if(connected){
            mi.insertUnitTests("unit_tests", result);
        }
        else{
            std::cout << "Failed to insert unit test results!" << std::endl;
            return 0;
        }
    }
*/
    return 0;
}