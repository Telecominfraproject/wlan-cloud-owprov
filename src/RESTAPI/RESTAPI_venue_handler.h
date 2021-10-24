//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//


#ifndef OWPROV_RESTAPI_VENUE_HANDLER_H
#define OWPROV_RESTAPI_VENUE_HANDLER_H

#include "framework/MicroService.h"
#include "StorageService.h"

namespace OpenWifi {
    class RESTAPI_venue_handler : public RESTAPIHandler {
    public:
        RESTAPI_venue_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer & Server, bool Internal)
        : RESTAPIHandler(bindings, L,
                         std::vector<std::string>{
            Poco::Net::HTTPRequest::HTTP_GET, Poco::Net::HTTPRequest::HTTP_POST,
            Poco::Net::HTTPRequest::HTTP_PUT, Poco::Net::HTTPRequest::HTTP_DELETE,
            Poco::Net::HTTPRequest::HTTP_OPTIONS},
            Server,
            Internal), DB_(StorageService()->VenueDB()) {}
        static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/venue/{uuid}"}; };
    private:
        VenueDB     &DB_;
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


#endif //OWPROV_RESTAPI_VENUE_HANDLER_H
