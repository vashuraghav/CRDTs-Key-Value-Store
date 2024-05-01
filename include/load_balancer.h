//
// Created by Manas on 4/30/24.
//

#ifndef CRDTS_KEY_VALUE_STORE_LOAD_BALANCER_H
#define CRDTS_KEY_VALUE_STORE_LOAD_BALANCER_H
#include <iostream>
#include "stdafx.h"
#include "../config/replica_config.h"
#include <future>
using namespace std;
using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;

class LoadBalancer {
public:
    LoadBalancer(utility::string_t url);
    virtual ~LoadBalancer();

    pplx::task<void> open() { return m_listener.open(); }
    pplx::task<void> close() { return m_listener.close(); }

    void get_replica_information(http_request req);

protected:
private:
    unsigned int numClientConnections;
    void populateReplicaConfigs();
    std::vector<ReplicaConfig> replicaConfigs;
    http_listener m_listener;
};

#endif //CRDTS_KEY_VALUE_STORE_LOAD_BALANCER_H
