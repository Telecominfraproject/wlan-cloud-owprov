//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//


#pragma once
#include "framework/RESTAPI_Handler.h"
#include "StorageService.h"

namespace OpenWifi {
    class RESTAPI_entity_list_handler : public RESTAPIHandler {
    public:
        RESTAPI_entity_list_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServerAccounting & Server, uint64_t TransactionId, bool Internal)
        : RESTAPIHandler(bindings, L,
                         std::vector<std::string>{
            Poco::Net::HTTPRequest::HTTP_GET,
            Poco::Net::HTTPRequest::HTTP_POST,
            Poco::Net::HTTPRequest::HTTP_OPTIONS},
            Server,
            TransactionId,
            Internal){}
        static auto PathName() { return std::list<std::string>{"/api/v1/entity"}; };
    private:
        EntityDB    &DB_=StorageService()->EntityDB();
        void DoGet() final;
        void DoPost() final ;
        void DoPut() final {};
        void DoDelete() final {};
    };
}
