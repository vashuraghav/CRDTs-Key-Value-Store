#include "../include/client.h"
#include "../statebased/map.hh"
#include <cpprest/json.h>
#include <cpprest/http_client.h>
#include <cpprest/uri.h>
#include <chrono>
#include <string>
#include "../utility/request_utilities.h"

using namespace web;
using namespace web::http;
using namespace web::http::client;

Client::Client(int32_t clientId) : m(clientId), clientId(clientId) {
    // Initialize client here if needed
}

Client::Client(utility::string_t url, int32_t clientId) : m(clientId), clientId(clientId), m_listener(url) {
    this->initialize_client_information();
    this->sync();
}

Client::~Client() {
}

json::value Client::get_state(std::string port) {
    // Implement logic to make a GET request to a single replica
    uri_builder builder(U("http://127.0.0.1:" + port));
    http_client client(builder.to_uri().to_string());

    // Send the GET request to the replica
    http_request syncRequest(methods::GET);
    http_response response = client.request(syncRequest).get();

    // Check if the request was successful
    if (response.status_code() == status_codes::OK) {
        // Extract the JSON response from the HTTP response
        json::value jsonResponse = response.extract_json().get();
        return jsonResponse;

    } else {
        // Handle the case when the request was not successful
        std::cerr << "Failed to get: " << response.status_code() << std::endl;
        return NULL;
    }
}

json::value Client::get_state(std::string ipAddress, std::string port) {
    // Implement logic to make a GET request to a single replica
    uri_builder builder(U("http://" + ipAddress + ":" + port));
    http_client client(builder.to_uri().to_string());

    // Send the GET request to the replica
    http_request syncRequest(methods::GET);
    syncRequest.set_body(json::value::object().serialize().c_str(), "application/json");

    http_response response = client.request(syncRequest).get();
    // Check if the request was successful
    if (response.status_code() == status_codes::OK) {
        // Extract the JSON response from the HTTP response
        json::value jsonResponse = response.extract_json().get();
        return jsonResponse;

    } else {
        // Handle the case when the request was not successful
        std::cerr << "Failed to get: " << response.status_code() << std::endl;
        return NULL;
    }
}

bool Client::can_connect_to_replica() {
    this->initialize_my_replica();
    json::value res = get_state(this->myReplicaConfig.ipAddress, std::to_string(this->myReplicaConfig.port));
    return res != NULL;
}

void Client::sync() {
    // Implement logic to make a POST request to a single replica
    while(!can_connect_to_replica());
    std::cout << "Connected to replica " << this->myReplicaConfig.ipAddress << " " << this->myReplicaConfig.port << std::endl;
    std::string replicaIpAddress = this->myReplicaConfig.ipAddress;
    int replicaPort = this->myReplicaConfig.port;
    utility::string_t addr = U("http://" + replicaIpAddress + ":");
    addr.append(U(std::to_string(replicaPort)));
    uri_builder builder(addr);
    http_client client(builder.to_uri().to_string());

    // Create JSON payload for the request if needed
    json::value requestJson = m.to_json();

    // Add any necessary data to the JSON object

    // Send the POST request to the replica
    http_request syncRequest(methods::POST);
    syncRequest.set_body(requestJson.serialize().c_str(), "application/json");
    http_response response = client.request(syncRequest).get();
    // std::cout << "Sync response:\n" << response.to_string() << std::endl;
     // Check if the request was successful
    if (response.status_code() == status_codes::OK) {
        // Extract the JSON response from the HTTP response
        json::value jsonResponse = response.extract_json().get();
        
        // Output the JSON response
        // std::cout << "JSON response:\n" << jsonResponse.serialize() << std::endl;

        Map<string, string> updated_map(jsonResponse);
        m.merge(updated_map);

    } else {
        // Handle the case when the request was not successful
        std::cerr << "Failed to sync: " << response.status_code() << std::endl;
    }
    cout<<" My final map "<<RequestUtilities::format_json(m.to_json(), 2)<<endl;
}

void Client::initialize_client_information() {
    // client initialization
    //  m.put("456", "yoyoyo");
    // cout<<" Receiving "<<m.get("456")<<endl;
    // cout<<"Serializing -> ";
    // cout<<m.to_json().serialize()<<endl;
    // cout<<"Deserializing -> ";
    // Map<string, string> m2(m.to_json());
    // cout<<endl;
    // cout<<m2.to_json().serialize()<<endl;
    // cout<<" Receiving "<<m2.get("456")<<endl;
}

void Client::initialize_my_replica() {
    // Implement logic to make a POST request to a single replica
    uri_builder builder(U("http://127.0.0.1:30000"));
    http_client loadBalancer(builder.to_uri().to_string());

    // Create JSON payload for the request if needed
    json::value requestJson = json::value::object();

    // Add any necessary data to the JSON object

    // Send the POST request to the replica
    http_request getMyReplicaRequest(methods::GET);
    getMyReplicaRequest.set_body(requestJson.serialize().c_str(), "application/json");
    std::cout << "Sending a request to Load Balancer to get replica to connect to" << std::endl;
    http_response response = loadBalancer.request(getMyReplicaRequest).get();
    std::cout << "Initialize myReplica info response:" << std::endl << response.to_string() << std::endl;
    // Check if the request was successful
    if (response.status_code() == status_codes::OK) {
        // Extract the JSON response from the HTTP response
        json::value jsonResponse = response.extract_json().get();

        this->myReplicaConfig.replicaId = jsonResponse.at(U("replicaId")).as_integer();
        this->myReplicaConfig.ipAddress = jsonResponse.at(U("ipAddress")).as_string();
        this->myReplicaConfig.port = jsonResponse.at(U("port")).as_integer();
        // Output the JSON response
        std::cout << "JSON response:\n" << jsonResponse.serialize() << std::endl;

    } else {
        // Handle the case when the request was not successful
        std::cerr << "Failed to get a replica to connect to: " << response.status_code() << std::endl;
    }
}

void Client::put_kv_in_map(string key, string value) {
    m.put(key, value);
}

string Client::get_key_from_map(string key){
    return m.get(key);
}

bool Client::check_contains(string key){
    return m.contains(key);
}

void Client::remove_key_from_map(string key) {
    m.remove(key);
}

void Client::trigger_sync() {
    this->sync();
}
