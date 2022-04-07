//
// Created by stephane bourque on 2022-04-06.
//

#ifndef OWPROV_RESTAPI_SERVICE_CLASS_LIST_HANDLER_H
#define OWPROV_RESTAPI_SERVICE_CLASS_LIST_HANDLER_H


class RESTAPI_service_class_list_handler {

};


#endif //OWPROV_RESTAPI_SERVICE_CLASS_LIST_HANDLER_H

#pragma once

#include "framework/MicroService.h"
#include "StorageService.h"

namespace OpenWifi {

    class RESTAPI_service_class_list_handler : public RESTAPIHandler {
    public:
        RESTAPI_service_class_list_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer & Server, uint64_t TransactionId, bool Internal)
                : RESTAPIHandler(bindings, L,
                                 std::vector<std::string>{
                                         Poco::Net::HTTPRequest::HTTP_GET,
                                         Poco::Net::HTTPRequest::HTTP_OPTIONS},
                                 Server,
                                 TransactionId,
                                 Internal) {
        }
        static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/serviceClasses"}; };
    private:
        ServiceClassDB    &DB_=StorageService()->ServiceClassDB();
        void DoGet() final;
        void DoPost() final {};
        void DoPut() final {};
        void DoDelete() final {};
    };
}
