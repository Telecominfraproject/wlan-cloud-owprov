//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//
#pragma once

#include "framework/MicroService.h"
#include "RESTObjects/RESTAPI_ProvObjects.h"
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
            DB_(StorageService()->ConfigurationDB()){}

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
