#include <iostream>

#include "utility/request_utilities.h"
#include "include/stdafx.h"
#include "include/handler.h"
#include "include/client.h"
#include "include/load_balancer.h"
#include <string>
#include <sstream>
using namespace std;
using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;




std::unique_ptr<handler> g_httpHandler;
std::unique_ptr<Client> g_httpClient;
std::unique_ptr<LoadBalancer> g_httpLoadBalancer;


void on_initialize_server(const string_t& address, const int32_t replicaId)
{


    uri_builder uri(address);
  

    auto addr = uri.to_uri().to_string();
     g_httpHandler = std::unique_ptr<handler>(new handler(addr, replicaId));
     g_httpHandler->open().wait();

    ucout << utility::string_t(U("Listening for requests at: ")) << addr << std::endl;

    return;
}

void on_initialize_loadbalancer(const string_t& address)
{
    uri_builder uri(address);
    auto addr = uri.to_uri().to_string();
    g_httpLoadBalancer = std::unique_ptr<LoadBalancer>(new LoadBalancer(addr));
    g_httpLoadBalancer->open().wait();

    ucout << utility::string_t(U("Listening for requests at: ")) << addr << std::endl;

    return;
}

void on_initialize_client(const string_t& address, const int32_t clientId) {
    uri_builder uri(address);
    auto addr = uri.to_uri().to_string();
    g_httpClient = std::unique_ptr<Client>(new Client(addr, clientId));
    g_httpClient->open().wait();

    ucout << utility::string_t(U("Client is live at: ")) << addr << std::endl;

    // Menu loop
    while (true) {
        // Display menu options
        ucout << "Menu Options:" << std::endl;
        ucout << "1. Map Put" << std::endl;
        ucout << "2. Map Get" << std::endl;
        ucout << "3. Map Remove" << std::endl;
        ucout << "4. Map Contains" << std::endl;
        ucout << "5. Sync with remote" << std::endl;
        ucout << "6. Exit" << std::endl;

        // Prompt user for choice
        ucout << "Enter your choice: ";
        int choice;
        std::cin >> choice;

        // Process user choice
        switch (choice) {
            case 1: {
                // Map Put operation
                ucout << "Enter key and value (separated by space): ";
                std::string key, value;
                std::cin >> key >> value;
                // Call a function to perform the put operation
                g_httpClient->put_kv_in_map(key, value);
                break;
            }
            case 2: {
                // Map Get operation
                ucout << "Enter key to get value: ";
                std::string key;
                std::cin >> key;
                // Call a function to perform the get operation
                if (g_httpClient->check_contains(key)) {
                    cout << g_httpClient->get_key_from_map(key) << endl;
                } else {
                    cout << "Key: " << key << " does not exist in K-V store" << endl;
                }
                break;
            }
            case 3: {
                // Map Remove operation
                ucout << "Enter key to remove: ";
                std::string key;
                std::cin >> key;
                // Call a function to perform the remove operation
                g_httpClient->remove_key_from_map(key);
                break;
            }
            case 4: {
                // Map Contains operation
                ucout << "Enter key to check: ";
                std::string key;
                std::cin >> key;
                // Call a function to perform the contains operation
                cout << g_httpClient->check_contains(key) << endl;
                break;
            }
            case 5: {
                g_httpClient->trigger_sync();
                break;
            }
            case 6:
                // Exit the program
                return;
            default:
                ucout << "Invalid choice. Please try again." << std::endl;
        }
    }
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

void on_shutdown_loadbalancer()
{
    g_httpLoadBalancer->close().wait();
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
    bool isLoadBalancer = isServer == 2;
    if(argc == 3 && !isLoadBalancer)
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
    } else {
        std::string configFileName("config/loadbalancer_properties.txt");
        std::ifstream file(configFileName);
        int lineNum = 0;
        if (file.is_open()) {
            std::cout << "Opened loadbalancer file\n";
            std::string line;
            while (std::getline(file, line)) {
                if (lineNum == 0) {
                    address = U("http://" + line + ":");
                } else {
                    port = U(line);
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
    if (isLoadBalancer)
        on_initialize_loadbalancer(address);
    else if(isServer)
        on_initialize_server(address, nodeId);
    else
        on_initialize_client(address, nodeId);

    std::cout << "Press ENTER to exit." << std::endl;

    std::string line;
    std::getline(std::cin, line);
    if (isLoadBalancer)
        on_shutdown_loadbalancer();
    else if(isServer)
        on_shutdown_server();
    else
        on_shutdown_client();

    return 0;
}
