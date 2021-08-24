//
// Created by stephane bourque on 2021-08-23.
//

#ifndef OWPROV_RESTAPI_VENUE_LIST_HANDLER_H
#define OWPROV_RESTAPI_VENUE_LIST_HANDLER_H


class RESTAPI_venue_list_handler {

};

#include "RESTAPI_handler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "RESTAPI_ProvObjects.h"

namespace OpenWifi {

    typedef std::vector<ProvObjects::Venue>   VenueVec;

    class RESTAPI_venue_list_handler : public RESTAPIHandler {
    public:
        RESTAPI_venue_list_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, bool Internal)
        : RESTAPIHandler(bindings, L,
                         std::vector<std::string>{
            Poco::Net::HTTPRequest::HTTP_GET,
            Poco::Net::HTTPRequest::HTTP_OPTIONS},
            Internal) {}
            void handleRequest(Poco::Net::HTTPServerRequest &request,
                               Poco::Net::HTTPServerResponse &response) override final;
        static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/venue"}; };

        void DoGet(Poco::Net::HTTPServerRequest &Request,
                   Poco::Net::HTTPServerResponse &Response);
    };
}

#endif //OWPROV_RESTAPI_VENUE_LIST_HANDLER_H
