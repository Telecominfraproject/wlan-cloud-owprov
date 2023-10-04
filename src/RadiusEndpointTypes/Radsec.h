//
// Created by stephane bourque on 2023-10-03.
//

#pragma once

#include <Poco/Net/IPAddress.h>
#include <Poco/Net/SocketAddress.h>
#include <framework/utils.h>
#include <framework/SubSystemServer.h>
#include <RESTObjects/RESTAPI_ProvObjects.h>

namespace OpenWifi {

    namespace Radsec {

        class OpenRoaming : public SubSystemServer {
        public:
            static auto instance() {
                static auto instance_ = new OpenRoaming;
                return instance_;
            }

            inline int Start() override {

                return 0;
            }

            inline void Stop() override {

            }

            inline bool Render(const OpenWifi::ProvObjects::RADIUSEndPoint &RE, const std::string &SerialNumber, Poco::JSON::Object &Result) {
                if(RE.UseGWProxy) {
                    Poco::JSON::Object  Auth, Acct, CoA;

                    Auth.set("host", RE.Index);
                    Auth.set("port", 1812 );
                    Auth.set("secret", RE.RadsecServers[0].Secret);

                    Acct.set("host", RE.Index);
                    Acct.set("port", 1813);
                    Acct.set("secret", RE.RadsecServers[0].Secret);
                    Acct.set("interval", RE.AccountingInterval);

                    CoA.set("host", RE.Index);
                    CoA.set("port", 3799);
                    CoA.set("secret", RE.RadsecServers[0].Secret);

                    Result.set("nas-identifier", RE.NasIdentifier.empty() ? SerialNumber : RE.NasIdentifier );
                    Result.set("authentication", Auth);
                    Result.set("accounting", Acct);
                    Result.set("dynamic-authorization", CoA);

                } else {

                }
                return false;
            }

        private:
            OpenRoaming() noexcept
                    : SubSystemServer("OpenRoaming_Raadsec", "RADSEC", "radsec") {
            }
        };

    }

    inline auto OpenRoaming_Radsec() { return Radsec::OpenRoaming::instance(); }

}