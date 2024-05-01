//
// Created by Manas on 4/28/24.
//

#ifndef CRDTS_KEY_VALUE_STORE_REQUEST_UTILITIES_H
#define CRDTS_KEY_VALUE_STORE_REQUEST_UTILITIES_H
#include <string>
#include <vector>
#include <cpprest/json.h>
using namespace web;
using namespace std;
#include "../config/replica_config.h"

class RequestUtilities {
public:
    static std::vector<std::string> splitString(const std::string& line);
    static std::string format_json(const json::value& jsonValue, int indentation );
    static bool compareJSONObjects(const json::object& obj1, const json::object& obj2);
    static bool compareRegisters(const json::value& json1, const json::value& json2);
    static std::vector<ReplicaConfig> retrieveReplicaConfigsFromProperties();
};


#endif //CRDTS_KEY_VALUE_STORE_REQUEST_UTILITIES_H
