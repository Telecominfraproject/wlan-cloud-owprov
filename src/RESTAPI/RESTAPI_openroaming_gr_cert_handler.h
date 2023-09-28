//
// Created by stephane bourque on 2023-09-11.
//

#pragma once
#include "StorageService.h"
#include "framework/RESTAPI_Handler.h"

namespace OpenWifi {
    class RESTAPI_openroaming_gr_cert_handler : public RESTAPIHandler {
    public:
        RESTAPI_openroaming_gr_cert_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L,
                                            RESTAPI_GenericServerAccounting &Server, uint64_t TransactionId,
                                            bool Internal)
                : RESTAPIHandler(bindings, L,
                                 std::vector<std::string>{Poco::Net::HTTPRequest::HTTP_GET,
                                                          Poco::Net::HTTPRequest::HTTP_DELETE,
                                                          Poco::Net::HTTPRequest::HTTP_POST,
                                                          Poco::Net::HTTPRequest::HTTP_OPTIONS},
                                 Server, TransactionId, Internal) {}
        static auto PathName() { return std::list<std::string>{"/api/v1/openroaming/globalreach/certificate/{account}/{id}"}; };

    private:
        using RecordType = ProvObjects::GLBLRCertificateInfo;
        GLBLRCertsDB &DB_ = StorageService()->GLBLRCertsDB();
        void DoGet() final;
        void DoPost() final;
        void DoPut() final {};
        void DoDelete() final;
    };
} // namespace OpenWifi
