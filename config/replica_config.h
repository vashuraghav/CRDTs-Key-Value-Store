#include <string>
#ifndef CRDTS_KEY_VALUE_STORE_REPLICA_CONFIG_H
#define CRDTS_KEY_VALUE_STORE_REPLICA_CONFIG_H
struct ReplicaConfig {
    size_t replicaId;
    std::string ipAddress;
    int port;
};
#endif