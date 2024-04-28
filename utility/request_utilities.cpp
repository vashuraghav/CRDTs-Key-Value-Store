//
// Created by Manas on 4/28/24.
//

#include "request_utilities.h"
#include <sstream>
std::vector<std::string> RequestUtilities::splitString(const std::string& line) {
    std::istringstream iss(line);
    std::vector<std::string> tokens;
    std::string token;
    while (std::getline(iss, token, ' ')) {
        tokens.push_back(token);
    }
    return tokens;
}