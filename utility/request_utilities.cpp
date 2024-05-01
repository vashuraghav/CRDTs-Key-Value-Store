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

std::string RequestUtilities::format_json(const web::json::value& jsonValue, int indentation = 4) {
    // Serialize the JSON value to a string
    std::stringstream ss;
    ss << jsonValue.serialize();

    // Parse the string again to apply indentation
    std::string line;
    std::string result;
    int indentLevel = 0;
    bool inQuotes = false;
    for (char c : ss.str()) {
        if (c == '{' || c == '[') {
            indentLevel++;
            result += c;
            result += '\n';
            result += std::string(indentation * indentLevel, ' ');
        } else if (c == '}' || c == ']') {
            indentLevel--;
            result += '\n';
            result += std::string(indentation * indentLevel, ' ');
            result += c;
        } else if (c == '"') {
            inQuotes = !inQuotes;
            result += c;
        } else if (c == ',' && !inQuotes) {
            result += c;
            result += '\n';
            result += std::string(indentation * indentLevel, ' ');
        } else {
            result += c;
        }
    }

    return result;
}