//
// Created by Manas on 4/30/24.
//

#include "../include/load_balancer.h"
#include "../utility/constants.h"
#include "../utility/request_utilities.h"
using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace web::http::experimental::listener;
using namespace experimental;
LoadBalancer::LoadBalancer(utility::string_t url) : m_listener(url) {
    this->replicaConfigs = RequestUtilities::retrieveReplicaConfigsFromProperties();
    this->numClientConnections = 0;
    m_listener.support(methods::GET, std::bind(&LoadBalancer::get_replica_information, this, std::placeholders::_1));
}

void LoadBalancer::get_replica_information(http_request request) {
    request.extract_json().then([request, this](json::value jsonValue) {
        try {
            // Process the JSON content
            std::cout << "Received JSON: " << jsonValue.serialize() << std::endl;
            http_response response(status_codes::OK);
            response.headers().set_content_type(U("application/json"));
            json::value resJson = json::value::object();
            int replicaId = this->numClientConnections % this->replicaConfigs.size();
            resJson[U("replicaId")] = replicaId;
            resJson[U("ipAddress")] = json::value::string(U(this->replicaConfigs[replicaId].ipAddress));
            resJson[U("port")] = this->replicaConfigs[replicaId].port;
            response.set_body(resJson);
            request.reply(response);
            ++this->numClientConnections;
        } catch (const std::exception& e) {
            // Handle exception: client did not send a JSON object
            std::cerr << "Error: " << e.what() << std::endl;
            request.reply(status_codes::InternalError, "Something went wrong!");
        }
    }).wait();
}

LoadBalancer::~LoadBalancer() {
    //dtor
}
