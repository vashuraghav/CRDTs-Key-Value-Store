//
// Created by Manas on 4/28/24.
//

#ifndef CRDTS_KEY_VALUE_STORE_REQUEST_UTILITIES_H
#define CRDTS_KEY_VALUE_STORE_REQUEST_UTILITIES_H
#include <string>
#include <vector>
#include <cpprest/json.h>
using namespace web;

class RequestUtilities {
public:
    static std::vector<std::string> splitString(const std::string& line);
    static std::string format_json(const json::value& jsonValue, int indentation );
};


#endif //CRDTS_KEY_VALUE_STORE_REQUEST_UTILITIES_H
