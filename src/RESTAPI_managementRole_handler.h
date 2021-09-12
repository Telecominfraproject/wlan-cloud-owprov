//
// Created by stephane bourque on 2021-08-26.
//

#ifndef OWPROV_RESTAPI_MANAGEMENTROLE_HANDLER_H
#define OWPROV_RESTAPI_MANAGEMENTROLE_HANDLER_H

#include "RESTAPI_handler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

namespace OpenWifi {
    class RESTAPI_managementRole_handler : public RESTAPIHandler {
    public:
        RESTAPI_managementRole_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, bool Internal)
        : RESTAPIHandler(bindings, L,
                         std::vector<std::string>{
            Poco::Net::HTTPRequest::HTTP_GET, Poco::Net::HTTPRequest::HTTP_POST,
            Poco::Net::HTTPRequest::HTTP_PUT, Poco::Net::HTTPRequest::HTTP_DELETE,
            Poco::Net::HTTPRequest::HTTP_OPTIONS},
            Internal) {}
        static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/managementRole/{uuid}"}; };

        void DoGet() final ;
        void DoPost() final ;
        void DoPut() final ;
        void DoDelete() final ;
    };
}

#endif //OWPROV_RESTAPI_MANAGEMENTROLE_HANDLER_H
