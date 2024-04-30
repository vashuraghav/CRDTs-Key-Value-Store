#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include "stdafx.h"
#include "../statebased/map.hh"
#include <future>

using namespace std;
using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;

class Client {
public:
    Client(int32_t clientId);
    Client(utility::string_t url, int32_t clientId);
    virtual ~Client();

    pplx::task<void> open() { return m_listener.open(); }
    pplx::task<void> close() { return m_listener.close(); }

    void put_kv_in_map(string key, string value);
    string get_key_from_map(string key);
    bool check_contains(string key);
    void remove_key_from_map(string key);
    void trigger_sync();

protected:
private:
    void sync();
    void initialize_client_information();
    int32_t clientId;
    http_listener m_listener;
    Map<string, string> m;
};

#endif