//
// Created by stephane bourque on 2021-08-30.
//

#ifndef OWPROV_SECURITYDBPROXY_H
#define OWPROV_SECURITYDBPROXY_H

#include "framework/OpenWifiTypes.h"
#include "RESTAPI/RESTAPI_SecurityObjects.h"
#include "framework/SubSystemServer.h"

namespace OpenWifi {
    class SecurityDBProxy : public SubSystemServer {
    public:
        static SecurityDBProxy *instance() {
            if (instance_ == nullptr) {
                instance_ = new SecurityDBProxy;
            }
            return instance_;
        }

        int Start() override;
        void Stop() override;

        bool Exists(const char *F, const Types::UUID_t & User);
        bool GetRecord(const char *F, const Types::UUID_t & User, SecurityObjects::UserInfo & UserInfo);
        bool UpdateCache(const std::string &UserUUID, SecurityObjects::UserInfo & UserInfo);
        const std::string Prefix() { return "usr"; };

    private:
        static SecurityDBProxy * instance_;
        struct CachedUInfo {
            SecurityObjects::UserInfo   UI;
            uint64_t                    LastUpdate=0;
        };

        std::map<std::string,CachedUInfo> Cache_;

        SecurityDBProxy() noexcept:
            SubSystemServer("SecurityProxy", "SEC-SVR", "securityproxy")
        {
        }
    };

    inline SecurityDBProxy * SecurityDBProxy() { return SecurityDBProxy::instance(); }

}

#endif //OWPROV_SECURITYDBPROXY_H
