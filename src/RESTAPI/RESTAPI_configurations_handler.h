//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//
#ifndef OWPROV_RESTAPI_CONFIGURATIONS_HANDLER_H
#define OWPROV_RESTAPI_CONFIGURATIONS_HANDLER_H

#include "framework/RESTAPI_handler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "RESTAPI_ProvObjects.h"
#include "StorageService.h"

namespace OpenWifi {
    class RESTAPI_configurations_handler : public RESTAPIHandler {
    public:
        RESTAPI_configurations_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer & Server, bool Internal)
        : RESTAPIHandler(bindings, L,
                         std::vector<std::string>{
            Poco::Net::HTTPRequest::HTTP_GET, Poco::Net::HTTPRequest::HTTP_POST,
            Poco::Net::HTTPRequest::HTTP_PUT, Poco::Net::HTTPRequest::HTTP_DELETE,
            Poco::Net::HTTPRequest::HTTP_OPTIONS},
            Server,
            Internal),
            DB_(Storage()->ConfigurationDB()){}

        static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/configurations/{uuid}"}; };

        void DoGet();
        void DoPost();
        void DoPut();
        void DoDelete();
    private:
        bool ValidateConfigBlock(const ProvObjects::DeviceConfiguration &Config, std::string & Error);
        ConfigurationDB     &DB_;
    };
}

#endif //OWPROV_RESTAPI_CONFIGURATIONS_HANDLER_H
