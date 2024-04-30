#include "../include/handler.h"
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
using namespace web::http::experimental::listener;
using namespace experimental;
handler::handler(int32_t replicaId): m(replicaId), replicaId(replicaId)
{
    //ctor
    //Reads in config to know how many other replicas are there
    //initialize your own replica ID
    //in each request make sure that if you see your own replica id, do not send or process that request.
}
handler::handler(utility::string_t url, int32_t replicaId): m(replicaId), replicaId(replicaId), m_listener(url)
{
    this->initialize_replica_information();
    m_listener.support(methods::GET, std::bind(&handler::handle_get, this, std::placeholders::_1));
    m_listener.support(methods::PUT, std::bind(&handler::handle_put, this, std::placeholders::_1));
    m_listener.support(methods::POST, std::bind(&handler::handle_post, this, std::placeholders::_1));
    m_listener.support(methods::DEL, std::bind(&handler::handle_delete, this, std::placeholders::_1));

}
handler::~handler()
{
    //dtor
}

void handler::initialize_replica_information() {
    string configFileName("config/server_properties.txt");

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
                this->replicaConfigs.push_back({i, ipAddresses[i], stoi(ports[i])});
            }
        } else {
            std::cerr << "REPLICA CONFIG IS INVALID: Experiments will fail." << std::endl;
        }
    } else {
        std::cerr << "Unable to open file: " << configFileName << std::endl;
    }
}
void handler::handle_error(pplx::task<void>& t)
{
    try
    {
        t.get();
    }
    catch(...)
    {
        // Ignore the error, Log it if a logger is available
    }
}


//
// Get Request 
//
void handler::handle_get(http_request message)
{
    ucout <<  message.to_string() << endl;

    auto paths = http::uri::split_path(http::uri::decode(message.relative_uri().path()));

    message.relative_uri().path();
	//Dbms* d  = new Dbms();
    //d->connect();

      concurrency::streams::fstream::open_istream(U("static/index.html"), std::ios::in).then([=](concurrency::streams::istream is)
    {
        message.reply(status_codes::OK, is,  U("text/html"))
		.then([](pplx::task<void> t)
		{
			try{
				t.get();
			}
			catch(...){
				//
			}
	});
    }).then([=](pplx::task<void>t)
	{
		try{
			t.get();
		}
		catch(...){
			message.reply(status_codes::InternalError,U("INTERNAL ERROR "));
		}
	});

    return;

};

// Define a helper function to handle asynchronous broadcast and response waiting
// this was undo function but not using it now.
    void handler::broadcast_all_and_wait_async(const std::vector<ReplicaConfig>& configs, json::value &reqJson, size_t expectedResponses) {
        std::vector<std::future<void>> futures;
        size_t successfulResponses = 0;
        reqJson[U("from_replica")] = json::value::boolean(true);
        const std::string requestBody = reqJson.serialize();
        for (const auto& replicaConfig : configs) {
            futures.push_back(std::async(std::launch::async, [replicaConfig, requestBody, &successfulResponses] {
                try {
                    utility::string_t addr = U("http://" + replicaConfig.ipAddress + ":" + std::to_string(replicaConfig.port));
                    uri_builder uri(addr);
                    http_client client(uri.to_uri().to_string());
                    http_request replicaRequest(methods::POST);
                    replicaRequest.set_body(requestBody, "application/json");
                    http_response res = client.request(replicaRequest).get();
                    std::cout << "Got response from " << replicaConfig.replicaId << ": " << res.to_string() << std::endl;
                    // Check if response is successful (you might want to refine this condition)
                    if (res.status_code() == status_codes::OK) {
                        ++successfulResponses;
                    }
                } catch (const json::json_exception& e) {
                    // Handle exception: client did not send a JSON object
                    std::cerr << "Error: " << e.what() << std::endl;
                }
            }));
        }

        // Wait for all futures to be ready
        for (auto& future : futures) {
            future.wait();
        }

        // Wait for a while to ensure all responses are received (you can adjust this duration)
        std::this_thread::sleep_for(100ms);

        // Check if we received enough successful responses
        if (successfulResponses >= expectedResponses) {
            std::cout << "Received enough undo successful responses: " << successfulResponses << std::endl;
            // Respond to the client or take further action
        } else {
            std::cout << "Did not receive undo enough successful responses: " << successfulResponses << std::endl;
            // Handle the case where expected number of responses were not received
        }
    }


bool handler::broadcast_and_wait_async(const std::vector<ReplicaConfig>& configs, json::value &reqJson, size_t expectedResponses) {
    std::vector<std::future<void>> futures;
    size_t successfulResponses = 0;
    reqJson[U("from_replica")] = json::value::boolean(true);
    const std::string requestBody = reqJson.serialize();
    std::mutex responseMutex;

    for (const auto& replicaConfig : configs) {
        futures.push_back(std::async(std::launch::async, [&replicaConfig, requestBody, &successfulResponses, &responseMutex, expectedResponses] {
            try {
                utility::string_t addr = U("http://" + replicaConfig.ipAddress + ":" + std::to_string(replicaConfig.port));
                uri_builder uri(addr);
                http_client client(uri.to_uri().to_string());
                http_request replicaRequest(methods::POST);
                replicaRequest.set_body(requestBody, "application/json");
                http_response res = client.request(replicaRequest).get();
                std::cout << "Got response from " << replicaConfig.replicaId << ": " << res.to_string() << std::endl;

                // Update successfulResponses atomically
                {
                    std::lock_guard<std::mutex> lock(responseMutex);
                    if (res.status_code() == status_codes::OK && successfulResponses < expectedResponses) {
                        ++successfulResponses;
                    }
                }

            } catch (const json::json_exception& e) {
                // Handle exception: client did not send a JSON object
                std::cerr << "Error: " << e.what() << std::endl;
            }
        }));
    }

    // Wait for all futures to be ready or until expected number of responses are received
    size_t futuresCompleted = 0;
    while (futuresCompleted < futures.size() && successfulResponses < expectedResponses) {
        futures[futuresCompleted].wait();
        ++futuresCompleted;
    }

    // Wait for a while to ensure all responses are received (you can adjust this duration)
    std::this_thread::sleep_for(100ms);

    // Check if we received enough successful responses
    if (successfulResponses >= expectedResponses) {
        std::cout << "Received enough successful responses: " << successfulResponses << std::endl;
        return true;
        // Respond to the client or take further action
    } else {
        std::cout << "Did not receive enough successful responses: " << successfulResponses << std::endl;
        // broadcast_all_and_wait_async(configs, existingJson, 5);
        // Handle the case where expected number of responses were not received
        return false;
    }
}


void handler::broadcast_request_to_replicas(json::value &reqJson) {
    std::vector <std::future<void>> futures;
    reqJson[U("from_replica")] = json::value::boolean(true);
    utility::stringstream_t ss;
    reqJson.serialize(ss);
    auto modifiedJsonString = ss.str();
    for (ReplicaConfig &replicaConfig : this->replicaConfigs) {
        std::cout << replicaConfig.replicaId << std::endl;
        if (replicaConfig.replicaId == this->replicaId)
            continue;
        utility::string_t addr = U("http://" + replicaConfig.ipAddress + ":");
        utility::string_t port = U(std::to_string(replicaConfig.port));
        addr.append(port);
        uri_builder uri(addr);
        cout<<uri.to_uri().to_string()<<endl;
        http_client client(uri.to_uri().to_string());
        http_request replicaRequest(methods::POST);
        replicaRequest.set_body(modifiedJsonString, "application/json");
        http_response res = client.request(replicaRequest).get();
        std::cout << "Got response from " << replicaConfig.replicaId << ": " << res.to_string() << std::endl;
    }
}


//
// A POST request
//
// assuming that post request body will be a JSON object (looks like a dictionary)
// EG: {key1: v1, key2: v2}
void handler::handle_post(http_request request)
{
    bool sync_done = false;
    request.extract_json().then([request, &sync_done, this](json::value jsonValue) {
        try {
            // Process the JSON content
            std::cout << "Received JSON: " << jsonValue.serialize() << std::endl;
            if (jsonValue.has_field(U("from_replica"))) {
                jsonValue.erase(U("from_replica"));
                 Map<string, string> updated_map(jsonValue);
                 m.merge(updated_map);
                 sync_done = true;
            }else{
                Map<string, string> updated_map(jsonValue);
                m.merge(updated_map);
                json::value update_json = m.to_json();
                sync_done = broadcast_and_wait_async(this->replicaConfigs, update_json, 2); // Wait for 2 successful responses
            }
            cout<<" My final map "<<m.to_json()<<endl;
        } catch (const json::json_exception& e) {
            // Handle exception: client did not send a JSON object
            std::cerr << "Error: " << e.what() << std::endl;
            request.reply(status_codes::BadRequest, "Invalid JSON object format");
        }
    }).wait();
    if(sync_done){
        http_response response(status_codes::OK);
        response.headers().set_content_type(U("application/json"));
        response.set_body(m.to_json());
        request.reply(response);
        return ;
    }else{
        http_response response(status_codes::InternalError); // Use an appropriate HTTP status code for failure
        response.headers().set_content_type(U("application/json"));
        json::value errorJson;
        errorJson[U("error")] = json::value::string(U("Failed to synchronize with replicas"));
        response.set_body(errorJson);
        request.reply(response);
        return ;
    }
};

//
// A DELETE request
//
void handler::handle_delete(http_request message)
{
     ucout <<  message.to_string() << endl;

        string rep = U("WRITE YOUR OWN DELETE OPERATION");
      message.reply(status_codes::OK,rep);
    return;
};


//
// A PUT request 
//
void handler::handle_put(http_request message)
{
    ucout <<  message.to_string() << endl;
     string rep = U("WRITE YOUR OWN PUT OPERATION");
     message.reply(status_codes::OK,rep);
    return;
};
