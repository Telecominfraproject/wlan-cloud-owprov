//
// Created by stephane bourque on 2021-08-17.
//

#ifndef OWPROV_RESTAPI_INVENTORY_LIST_HANDLER_H
#define OWPROV_RESTAPI_INVENTORY_LIST_HANDLER_H


#include "RESTAPI_handler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

namespace OpenWifi {
    class RESTAPI_inventory_list_handler : public uCentral::RESTAPIHandler {
    public:
        RESTAPI_inventory_list_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, bool Internal)
        : RESTAPIHandler(bindings, L,
                         std::vector<std::string>{
            Poco::Net::HTTPRequest::HTTP_GET,
            Poco::Net::HTTPRequest::HTTP_OPTIONS},
            Internal) {}
            void handleRequest(Poco::Net::HTTPServerRequest &request,
                               Poco::Net::HTTPServerResponse &response) override final;
        static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/inventory"}; };

        void DoGet(Poco::Net::HTTPServerRequest &Request,
                   Poco::Net::HTTPServerResponse &Response);
    };
}

#endif //OWPROV_RESTAPI_INVENTORY_LIST_HANDLER_H
