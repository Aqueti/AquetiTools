


#include <Timer.h>
#include <Thread.h>
#include <MultiThread.h>
#include <ThreadPool.h>
//#include <LruCache.cpp>
//#include <TSMap.cpp>
#include <TSMap.tcc>
#include <aquetitools/revision.h>
#include <iostream>
#include <string.h>
//#include <TSQueue.cpp>
#include <TSQueue.tcc>
#include "JsonBox.h"


#include "ThreadPool.h"
#include <CRC.hpp>


#include "AquetiToolsTest.h"

using namespace std;

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

    return jsonReturn;
}
