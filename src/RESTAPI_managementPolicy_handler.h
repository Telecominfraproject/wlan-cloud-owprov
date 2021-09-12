//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//


#ifndef OWPROV_RESTAPI_MANAGEMENTPOLICY_HANDLER_H
#define OWPROV_RESTAPI_MANAGEMENTPOLICY_HANDLER_H

#include "RESTAPI_handler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

namespace OpenWifi {
    class RESTAPI_managementPolicy_handler : public RESTAPIHandler {
    public:
        RESTAPI_managementPolicy_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, bool Internal)
        : RESTAPIHandler(bindings, L,
                         std::vector<std::string>{
            Poco::Net::HTTPRequest::HTTP_GET, Poco::Net::HTTPRequest::HTTP_POST,
            Poco::Net::HTTPRequest::HTTP_PUT, Poco::Net::HTTPRequest::HTTP_DELETE,
            Poco::Net::HTTPRequest::HTTP_OPTIONS},
            Internal) {}
        static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/managementPolicy/{uuid}"}; };

        void DoGet();
        void DoPost();
        void DoPut();
        void DoDelete();
    };
}

#endif //OWPROV_RESTAPI_MANAGEMENTPOLICY_HANDLER_H
