#include <iostream>

#include "include/stdafx.h"
#include "include/handler.h"
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

void on_initialize(const string_t& address, const int32_t replicaId)
{


    uri_builder uri(address);
  

    auto addr = uri.to_uri().to_string();
     g_httpHandler = std::unique_ptr<handler>(new handler(addr, replicaId));
     g_httpHandler->open().wait();

    ucout << utility::string_t(U("Listening for requests at: ")) << addr << std::endl;

    return;
}

void on_shutdown()
{
     g_httpHandler->close().wait();
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
    int32_t serverId = 0;
    if(argc == 2)
    {
        string configFileName("config/server_properties.txt");
        serverId = stoi(argv[1]);
        std::ifstream file(configFileName);
        int lineNum = 0;
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                if (lineNum == 0) {
                    std::vector<std::string> ipAddresses = RequestUtilities::splitString(line);
                    std::cout << "Read in " << ipAddresses[serverId] << std::endl;
                    address = U("http://" + ipAddresses[serverId] + ":");
                } else {
                    std::vector<std::string> ports = RequestUtilities::splitString(line);
                    std::cout << "Read in " << ports[serverId] << std::endl;
                    port = U(ports[serverId]);
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

    on_initialize(address, serverId);
    std::cout << "Press ENTER to exit." << std::endl;

    std::string line;
    std::getline(std::cin, line);

    on_shutdown();
    return 0;
}
