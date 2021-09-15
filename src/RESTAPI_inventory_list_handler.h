//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//


#ifndef OWPROV_RESTAPI_INVENTORY_LIST_HANDLER_H
#define OWPROV_RESTAPI_INVENTORY_LIST_HANDLER_H


#include "RESTAPI_handler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "RESTAPI_ProvObjects.h"

namespace OpenWifi {

    class RESTAPI_inventory_list_handler : public RESTAPIHandler {
    public:
        RESTAPI_inventory_list_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer & Server, bool Internal)
        : RESTAPIHandler(bindings, L,
                         std::vector<std::string>{
            Poco::Net::HTTPRequest::HTTP_GET,
            Poco::Net::HTTPRequest::HTTP_OPTIONS},
            Server,
            Internal) {}
        static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/inventory"}; };

        void DoGet() final;
        void DoPost() final {};
        void DoPut() final {};
        void DoDelete() final {};

        void SendList(const ProvObjects::InventoryTagVec & Tags, bool SerialOnly);
    };
}

#endif //OWPROV_RESTAPI_INVENTORY_LIST_HANDLER_H
