//
// Created by stephane bourque on 2021-07-10.
//

#pragma once

#include "../framework/MicroService.h"

namespace OpenWifi {
    class RESTAPI_asset_server : public RESTAPIHandler {
    public:
        RESTAPI_asset_server(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer &Server, uint64_t TransactionId, bool Internal)
                : RESTAPIHandler(bindings, L,
                                 std::vector<std::string>
                                         {
                                          Poco::Net::HTTPRequest::HTTP_GET,
                                          Poco::Net::HTTPRequest::HTTP_OPTIONS},
                                          Server,
                                          TransactionId,
                                          Internal, false) {}
        static auto PathName() { return std::list<std::string>{"/wwwassets/{id}"}; };
        void DoGet() final;
        void DoPost() final {};
        void DoDelete() final {};
        void DoPut() final {};

    private:

    };
}

