//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#pragma once

#include "framework/MicroService.h"
#include "StorageService.h"

namespace OpenWifi {
    class RESTAPI_venue_handler : public RESTAPIHandler {
    public:
        RESTAPI_venue_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer & Server, uint64_t TransactionId, bool Internal)
        : RESTAPIHandler(bindings, L,
                         std::vector<std::string>{
            Poco::Net::HTTPRequest::HTTP_GET, Poco::Net::HTTPRequest::HTTP_POST,
            Poco::Net::HTTPRequest::HTTP_PUT, Poco::Net::HTTPRequest::HTTP_DELETE,
            Poco::Net::HTTPRequest::HTTP_OPTIONS},
            Server,
            TransactionId,
            Internal){}
        static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/venue/{uuid}"}; };
    private:
        VenueDB     &DB_=StorageService()->VenueDB();
        void DoGet() final;
        void DoPost() final;
        void DoPut() final;
        void DoDelete() final;

        template <typename T> bool IdExists(T &DB, const std::string &Field, const std::string &Error) {
            if(!Field.empty() && !DB.Exists("id",Field)) {
                BadRequest(Error);
                return false;
            }
            return true;
        }
    };
}
