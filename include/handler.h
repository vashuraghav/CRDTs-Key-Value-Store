#ifndef HANDLER_H
#define HANDLER_H
#include <iostream>
#include "stdafx.h"
//#include "../dbms/include/Dbms.h"
#include "../statebased/map.hh"
#include "../config/replica_config.h"
#include <future>

using namespace std;
using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;


class handler
{
    public:
        handler(int32_t replicaId);
        handler(utility::string_t url, int32_t replicaId);
        virtual ~handler();

        pplx::task<void>open(){return m_listener.open();}
        pplx::task<void>close(){return m_listener.close();}

    protected:

    private:
        void handle_get(http_request message);
        void handle_put(http_request message);
        void handle_post(http_request message);
        void handle_delete(http_request message);
        void handle_error(pplx::task<void>& t);
        void initialize_replica_information();
//        std::vector<std::future<void> > broadcast_request_to_replicas(json::value &reqJsonValue);
        void broadcast_request_to_replicas(json::value &reqJsonValue);
        vector<ReplicaConfig> replicaConfigs;
        int32_t replicaId;
        http_listener m_listener;
        Map<string, string> m; // should this be in handler class? feels like bad design. can move it out once functionality works.
};

#endif // HANDLER_H


