#include "AquetiToolsTest.h"

using namespace std;
std::vector<std::string> testList{"Timer", "CRC", "Thread", "MultiThread", "ThreadPool", "LruCache", "TSMap", "TSQueue"};

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
        } else if (!strcmp(argv[i], "--version")) {
            //AquetiTool::printVersion();
            return 1;
        } else if(!strcmp(argv[i], "-t")) {
            if (testAll) {
                testList.clear();
                testAll = false;
            }
            i++;
            testList.push_back( argv[i]);
        } else if(strcmp(argv[i], "-n") == 0){
            insert = false;
        } else if(strcmp(argv[i], "-s") == 0){
            testSubmodules = false;
        }
    }

    //run tests
    std::cout << "Testing AquetiTools..." << std::endl;
    JsonBox::Value result = testAquetiTools(testList, testSubmodules, valgrind);
    std::cout << result << std::endl;
    std::cout << "All tests completed!" << std::endl;

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