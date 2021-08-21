//
// Created by stephane bourque on 2021-08-16.
//

#ifndef OWPROV_RESTAPI_VENUE_HANDLER_H
#define OWPROV_RESTAPI_VENUE_HANDLER_H

#include "RESTAPI_handler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

namespace OpenWifi {
    class RESTAPI_venue_handler : public RESTAPIHandler {
    public:
        RESTAPI_venue_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, bool Internal)
        : RESTAPIHandler(bindings, L,
                         std::vector<std::string>{
            Poco::Net::HTTPRequest::HTTP_GET, Poco::Net::HTTPRequest::HTTP_POST,
            Poco::Net::HTTPRequest::HTTP_PUT, Poco::Net::HTTPRequest::HTTP_DELETE,
            Poco::Net::HTTPRequest::HTTP_OPTIONS},
            Internal) {}
            void handleRequest(Poco::Net::HTTPServerRequest &request,
                               Poco::Net::HTTPServerResponse &response) override final;
        static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/contact/{uuid}"}; };

        void DoGet(Poco::Net::HTTPServerRequest &Request,
                   Poco::Net::HTTPServerResponse &Response);
        void DoPost(Poco::Net::HTTPServerRequest &Request,
                    Poco::Net::HTTPServerResponse &Response);
        void DoPut(Poco::Net::HTTPServerRequest &Request,
                   Poco::Net::HTTPServerResponse &Response);
        void DoDelete(Poco::Net::HTTPServerRequest &Request,
                      Poco::Net::HTTPServerResponse &Response);
    };
}


#endif //OWPROV_RESTAPI_VENUE_HANDLER_H
