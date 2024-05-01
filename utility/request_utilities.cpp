//
// Created by Manas on 4/28/24.
//

#include "request_utilities.h"
#include <fstream>
#include <sstream>
#include "constants.h"
#include <iostream>
#include <vector>
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

bool RequestUtilities::compareJSONObjects(const json::object& obj1, const json::object& obj2) {
    if (obj1.size() != obj2.size()) {
        return false;
    }

    for (const auto& pair : obj1) {
        const auto& key = pair.first;
        const auto& val1 = pair.second;
        if (obj2.find(key) == obj2.end()) {
            return false;
        }

        const auto& val2 = obj2.at(key);

        if (val1 != val2) {
            return false;
        }
    }

    return true;
}

bool RequestUtilities::compareRegisters(const json::value& json1, const json::value& json2) {
    if (!json1.is_object() || !json2.is_object()) {
        return false;
    }

    const auto& reg1 = json1.at(U("_registers"));
    const auto& reg2 = json2.at(U("_registers"));

    if (!reg1.is_object() || !reg2.is_object()) {
        return false;
    }

    json::object reg1Filtered = reg1.as_object();
    json::object reg2Filtered = reg2.as_object();
    for (auto& reg : { &reg1Filtered, &reg2Filtered }) {
        for (auto& regField : *reg) {
            auto& fieldValue = regField.second;
            if (fieldValue.is_object() && fieldValue.has_field(U("timestamp"))) {
                auto& timestampObj = fieldValue.at(U("timestamp"));
                if (timestampObj.is_object() && timestampObj.has_field(U("replica_id"))) {
                    timestampObj.as_object().erase(U("replica_id"));
                }
            }
        }
    }

    return compareJSONObjects(reg1Filtered, reg2Filtered);
}

std::vector<ReplicaConfig> RequestUtilities::retrieveReplicaConfigsFromProperties() {
    std::string configFileName(REPLICA_CONFIG_FILENAME);
    std::vector<ReplicaConfig> replicaConfigs;
    std::ifstream file(configFileName);
    int lineNum = 0;
    if (file.is_open()) {
        std::string line;
        std::vector<std::string> ipAddresses;
        std::vector<std::string> ports;
        while (std::getline(file, line)) {
            if (lineNum == 0) {
                ipAddresses = RequestUtilities::splitString(line);
            } else {
                ports = RequestUtilities::splitString(line);
            }
            lineNum++;
        }
        file.close();
        if (ipAddresses.size() > 0 && ports.size() > 0 && ipAddresses.size() == ports.size()) {
            for (size_t i = 0; i < ipAddresses.size(); ++i) {
                replicaConfigs.push_back({i, ipAddresses[i], stoi(ports[i])});
            }
        } else {
            std::cerr << "REPLICA CONFIG IS INVALID: Experiments will fail." << std::endl;
        }
    } else {
        std::cerr << "Unable to open file: " << configFileName << std::endl;
    }
    return replicaConfigs;
}