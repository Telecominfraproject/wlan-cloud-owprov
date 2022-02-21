//
// Created by stephane bourque on 2022-02-20.
//

#pragma once

#include "framework/MicroService.h"

namespace OpenWifi {
    class RESTAPI_signup_handler : public RESTAPIHandler {
    public:
        RESTAPI_signup_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer & Server, uint64_t TransactionId, bool Internal)
                : RESTAPIHandler(bindings, L,
                                 std::vector<std::string>{
                                         Poco::Net::HTTPRequest::HTTP_POST,
                                         Poco::Net::HTTPRequest::HTTP_OPTIONS,
                                         Poco::Net::HTTPRequest::HTTP_PUT},
                                 Server,
                                 TransactionId,
                                 Internal, false, true ){}

        static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/signup"}; };

/*        inline bool RoleIsAuthorized(std::string & Reason) {
            if(UserInfo_.userinfo.userRole != SecurityObjects::USER_ROLE::SUBSCRIBER) {
                Reason = "User must be a subscriber";
                return false;
            }
            return true;
        }
*/
        void DoGet() final {};
        void DoPost() final;
        void DoPut() final ;
        void DoDelete() final {};
    private:

    };
}
