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