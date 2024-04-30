#include <iostream>

#include "include/stdafx.h"
#include "include/handler.h"
#include "include/client.h"
#include <string>
#include <fstream>
#include <sstream>
#include "utility/request_utilities.h"
using namespace std;
using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;




std::unique_ptr<handler> g_httpHandler;
std::unique_ptr<Client> g_httpClient;


void on_initialize_server(const string_t& address, const int32_t replicaId)
{


    uri_builder uri(address);
  

    auto addr = uri.to_uri().to_string();
     g_httpHandler = std::unique_ptr<handler>(new handler(addr, replicaId));
     g_httpHandler->open().wait();

    ucout << utility::string_t(U("Listening for requests at: ")) << addr << std::endl;

    return;
}

void on_initialize_client(const string_t& address, const int32_t clientId)
{


    uri_builder uri(address);
  

    auto addr = uri.to_uri().to_string();
     g_httpClient = std::unique_ptr<Client>(new Client(addr, clientId));
     g_httpClient->open().wait();

    ucout << utility::string_t(U("Client is live at: ")) << addr << std::endl;
    
    return;
}

void on_shutdown_server()
{
     g_httpHandler->close().wait();
    return;
}

void on_shutdown_client()
{
     g_httpClient->close().wait();
    return;
}

#ifdef _WIN32
int wmain(int argc, wchar_t *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    utility::string_t port = U("34568");
    utility::string_t address = U("http://127.0.0.1:");
    int32_t nodeId = 0;
    uint8_t isServer = stoi(argv[1]);
    if(argc == 3)
    {
        std::string configFileName;
        if (isServer) {
            configFileName = "config/server_properties.txt";
        } else {
            configFileName = "config/client_properties.txt";
        }
        nodeId = stoi(argv[2]);
        int nodeIndex = isServer? nodeId : nodeId-5;
        std::ifstream file(configFileName);
        int lineNum = 0;
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                if (lineNum == 0) {
                    std::vector<std::string> ipAddresses = RequestUtilities::splitString(line);
                    std::cout << "Read in " << ipAddresses[nodeIndex] << std::endl;
                    address = U("http://" + ipAddresses[nodeIndex] + ":");
                } else {
                    std::vector<std::string> ports = RequestUtilities::splitString(line);
                    std::cout << "Read in " << ports[nodeIndex] << std::endl;
                    port = U(ports[nodeIndex]);
                }
                lineNum++;
            }
            file.close();
            std::cout << "Closed file\n";
        } else {
            std::cerr << "Unable to open file: " << configFileName << std::endl;
        }
    }

    address.append(port);

    if(isServer)
        on_initialize_server(address, nodeId);
    else
        on_initialize_client(address, nodeId);

    std::cout << "Press ENTER to exit." << std::endl;

    std::string line;
    std::getline(std::cin, line);

    if(isServer)
        on_shutdown_server();
    else
        on_shutdown_client();

    return 0;
}
