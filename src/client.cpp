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

void Client::sync() {
    // Implement logic to make a POST request to a single replica
    uri_builder builder(U("http://127.0.0.1:8081"));
    http_client client(builder.to_uri().to_string());

    // Create JSON payload for the request if needed
    json::value requestJson = m.to_json();

    // Add any necessary data to the JSON object

    // Send the POST request to the replica
    http_request syncRequest(methods::POST);
    syncRequest.set_body(requestJson.serialize().c_str(), "application/json");
    http_response res = client.request(syncRequest).get();
    std::cout << "Sync response:\n" << res.to_string() << endl;
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
