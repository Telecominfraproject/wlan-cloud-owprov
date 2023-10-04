//
// Created by stephane bourque on 2023-09-28.
//

#pragma once

#include <Poco/Net/IPAddress.h>
#include <Poco/Net/SocketAddress.h>
#include <framework/utils.h>
#include <framework/SubSystemServer.h>

namespace OpenWifi {

    namespace Orion {

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

            static inline const std::vector<Utils::HostNameServerResult> OrionWifiServerAddresses = {
                    {"216.239.32.91", 2083},
                    {"216.239.34.91", 2083}
            };

            inline std::vector<Utils::HostNameServerResult> GetServers() {
                return OrionWifiServerAddresses;
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
                    : SubSystemServer("OpenRoaming_Orion", "ORION", "orion") {
            }
        };

    }

    inline auto OpenRoaming_Orion() { return Orion::OpenRoaming::instance(); }

}