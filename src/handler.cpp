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

//    return futures;

}
//
// A POST request
//
// assuming that post request body will be a JSON object (looks like a dictionary)
// EG: {key1: v1, key2: v2}
void handler::handle_post(http_request request)
{
    request.extract_json().then([request, this](json::value jsonValue) {
        try {
            // Process the JSON content
            std::cout << "Received JSON: " << jsonValue.serialize() << std::endl;
            if (jsonValue.has_field(U("from_replica"))) {
                jsonValue.erase(U("from_replica"));
                 Map<string, string> updated_map(jsonValue);
                 m.merge(updated_map);
            }else{
                Map<string, string> updated_map(jsonValue);
                m.merge(updated_map);
                json::value update_json = m.to_json();
                this->broadcast_request_to_replicas(update_json);
            }
            cout<<" My final map "<<m.to_json()<<endl;
//            const std::vector<std::future<void> >& futures = this->broadcast_request_to_replicas(jsonValue);
            //put a is_replica flag as true in the request sent to the other replicas.
            // Wait for f out of n responses from the replicas


//            std::vector<std::future<void>> responses;
//            for (auto& future : futures) {
//                responses.push_back(future);
//            }
//            std::atomic<int> received(0);
//
//            for (auto& response : futures) {
//                response.then([&received](http_response response) {
//                    if (response.status_code() == status_codes::OK) {
//                        received++;
//                    }
//                }).wait();
//            }
//            // Respond to the client after receiving f responses
//            if (received >= 2) {
//                request.reply(status_codes::OK, "Received and processed JSON object");
//            } else {
//                request.reply(status_codes::InternalError, "Failed to receive required responses from replicas");
//            }
        } catch (const json::json_exception& e) {
            // Handle exception: client did not send a JSON object
            std::cerr << "Error: " << e.what() << std::endl;
            request.reply(status_codes::BadRequest, "Invalid JSON object format");
        }
    }).wait();
    http_response response(status_codes::OK);
    response.headers().set_content_type(U("application/json"));
    response.set_body(m.to_json());
    request.reply(response);
    return ;
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
