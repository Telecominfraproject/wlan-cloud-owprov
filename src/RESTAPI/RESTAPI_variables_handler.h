//
// Created by stephane bourque on 2022-02-23.
//
#pragma once

#include "framework/MicroService.h"
#include "StorageService.h"

namespace OpenWifi {
    class RESTAPI_variables_handler : public RESTAPIHandler {
    public:
        RESTAPI_variables_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer & Server, uint64_t TransactionId, bool Internal)
                : RESTAPIHandler(bindings, L,
                                 std::vector<std::string>{
                                         Poco::Net::HTTPRequest::HTTP_GET, Poco::Net::HTTPRequest::HTTP_POST,
                                         Poco::Net::HTTPRequest::HTTP_PUT, Poco::Net::HTTPRequest::HTTP_DELETE,
                                         Poco::Net::HTTPRequest::HTTP_OPTIONS},
                                 Server,
                                 TransactionId,
                                 Internal){}
        static auto PathName() { return std::list<std::string>{"/api/v1/variable/{uuid}"}; };
    private:
        VariablesDB    & DB_=StorageService()->VariablesDB();
        void DoGet() final ;
        void DoPost() final ;
        void DoPut() final ;
        void DoDelete() final ;
    };
}
