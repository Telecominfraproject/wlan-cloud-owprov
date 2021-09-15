//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//


#ifndef OWPROV_RESTAPI_ENTITY_LIST_HANDLER_H
#define OWPROV_RESTAPI_ENTITY_LIST_HANDLER_H

#include "RESTAPI_handler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

namespace OpenWifi {
    class RESTAPI_entity_list_handler : public RESTAPIHandler {
    public:
        RESTAPI_entity_list_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer & Server, bool Internal)
        : RESTAPIHandler(bindings, L,
                         std::vector<std::string>{
            Poco::Net::HTTPRequest::HTTP_GET,
            Poco::Net::HTTPRequest::HTTP_POST,
            Poco::Net::HTTPRequest::HTTP_OPTIONS},
            Server,
            Internal) {}
        static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/entity"}; };

        void DoGet() final;
        void DoPost() final ;
        void DoPut() final {};
        void DoDelete() final {};
    };
}

#endif //OWPROV_RESTAPI_ENTITY_LIST_HANDLER_H
