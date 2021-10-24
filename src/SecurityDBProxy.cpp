//
// Created by stephane bourque on 2021-08-30.
//

#include "SecurityDBProxy.h"
#include "framework/MicroService.h"
#include "RESTObjects/RESTAPI_SecurityObjects.h"

namespace OpenWifi {
    class SecurityDBProxy *SecurityDBProxy::instance_ = nullptr;

    int SecurityDBProxy::Start() {
        return 0;
    }

    void SecurityDBProxy::Stop() {

    }

    bool SecurityDBProxy::UpdateCache(const std::string &UserUUID, SecurityObjects::UserInfo & UserInfo) {
        try {
            auto CachedUser = Cache_.find(UserUUID);
            uint64_t Now=std::time(nullptr);
            if(CachedUser==Cache_.end() || (Now-CachedUser->second.LastUpdate)>120) {
                Types::StringPairVec QueryData;
                OpenAPIRequestGet	Req(    uSERVICE_SECURITY,
                                             "/api/v1/user/" + UserUUID,
                                             QueryData,
                                             5000);

                Poco::JSON::Object::Ptr Response;
                auto StatusCode = Req.Do(Response);
                if( StatusCode == Poco::Net::HTTPResponse::HTTP_OK) {
                    CachedUInfo UI;
                    if(UI.UI.from_json(Response)) {
                        UserInfo = UI.UI;
                        UI.LastUpdate=Now;
                        Cache_[UserUUID]=UI;
                        return true;
                    }
                } else {
                }
            } else {
                return true;
            }
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        return false;
    }

    bool SecurityDBProxy::Exists(const char *F, const Types::UUID_t & User) {
        SecurityObjects::UserInfo UserInfo;
        return UpdateCache(User,UserInfo);
    }

    bool SecurityDBProxy::GetRecord(const char *F, const Types::UUID_t & User, SecurityObjects::UserInfo & UserInfo) {
        return UpdateCache(User, UserInfo);
    }

}
