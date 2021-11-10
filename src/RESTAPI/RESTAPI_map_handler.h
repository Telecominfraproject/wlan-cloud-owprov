//
// Created by stephane bourque on 2021-11-09.
//

#ifndef OWPROV_RESTAPI_MAP_HANDLER_H
#define OWPROV_RESTAPI_MAP_HANDLER_H

#include "framework/MicroService.h"
#include "StorageService.h"

namespace OpenWifi {
    class RESTAPI_map_handler : public RESTAPIHandler {
    public:
        RESTAPI_map_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer & Server, bool Internal)
        : RESTAPIHandler(bindings, L,
                         std::vector<std::string>{
            Poco::Net::HTTPRequest::HTTP_GET, Poco::Net::HTTPRequest::HTTP_POST,
            Poco::Net::HTTPRequest::HTTP_PUT, Poco::Net::HTTPRequest::HTTP_DELETE,
            Poco::Net::HTTPRequest::HTTP_OPTIONS},
            Server,
            Internal),
            DB_(StorageService()->MapDB()){}
            static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/map/{uuid}"}; };
    private:
        MapDB    &DB_;
        void DoGet() final ;
        void DoPost() final ;
        void DoPut() final ;
        void DoDelete() final ;
    };
}


#endif //OWPROV_RESTAPI_MAP_HANDLER_H
