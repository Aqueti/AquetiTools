
#include "AquetiToolsTest.h"

JsonBox::Value testAquetiTools(std::vector<std::string> unitList, bool testSubmodules, bool valgrind)
{
    JsonBox::Value jsonReturn;
    JsonBox::Value jsonUnits;
    JsonBox::Value jsonValue;
    bool pass = true;

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

    //Get results from units
    for(std::vector<std::string>::iterator it = unitList.begin(); it != unitList.end(); ++it ) {
        if( !it->compare("Timer")) {
              if(valgrind) {
                std::cout << "Timer will not be tested." << std::endl;
            } 
            else {
                std::cout << "Testing Timer..." <<std::endl;
                jsonValue = atl::testTimer(true, false);
                jsonUnits["Timer"] = jsonValue;
                jsonReturn["units"] = jsonUnits;
                if(jsonValue["pass"].getBoolean()){
                    std::cout << "Timer passed successfully!" << std::endl;
                    pass = pass && true;
                }
                else{
                    std::cout << "Timer failed to pass!" << std::endl;
                    pass = pass && false;
                }
            }
        }/*
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
        }*/
    }

    //get pass
    jsonReturn["pass"] = pass;
    return jsonReturn;
}
