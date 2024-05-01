//
// Created by Manas on 4/28/24.
//

#ifndef CRDTS_KEY_VALUE_STORE_REQUEST_UTILITIES_H
#define CRDTS_KEY_VALUE_STORE_REQUEST_UTILITIES_H
#include <string>
#include <vector>
#include "../config/replica_config.h"

class RequestUtilities {
public:
    static std::vector<std::string> splitString(const std::string& line);
    static std::vector<ReplicaConfig> retrieveReplicaConfigsFromProperties();
};


#endif //CRDTS_KEY_VALUE_STORE_REQUEST_UTILITIES_H
