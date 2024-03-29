//
// Created by stephane bourque on 2023-10-18.
//

#pragma once

#include <Poco/Net/IPAddress.h>
#include <Poco/Net/SocketAddress.h>
#include <framework/utils.h>
#include <framework/SubSystemServer.h>
#include <RESTObjects/RESTAPI_ProvObjects.h>

namespace OpenWifi {

    namespace GenericRadius {
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

            inline bool Render(const OpenWifi::ProvObjects::RADIUSEndPoint &RE, const std::string &SerialNumber,
                               Poco::JSON::Object &Result) {
                if (RE.UseGWProxy) {
                    Poco::JSON::Object Auth, Acct, CoA;

                    Auth.set("host", RE.Index);
                    Auth.set("port", RE.RadiusServers[0].Authentication[0].Port);
                    Auth.set("secret", RE.RadiusServers[0].Authentication[0].Secret);

                    Acct.set("host", RE.Index);
                    Acct.set("port", RE.RadiusServers[0].Accounting[0].Port);
                    Acct.set("secret", RE.RadiusServers[0].Accounting[0].Secret);
                    Acct.set("interval", RE.AccountingInterval);

                    CoA.set("host", RE.Index);
                    CoA.set("port", RE.RadiusServers[0].CoA[0].Port);
                    CoA.set("secret", RE.RadiusServers[0].CoA[0].Secret);

                    Result.set("nas-identifier", RE.NasIdentifier.empty() ? SerialNumber : RE.NasIdentifier);
                    Result.set("authentication", Auth);
                    Result.set("accounting", Acct);
                    Result.set("dynamic-authorization", CoA);
                } else {

                }
                return false;
            }

        private:
            OpenRoaming() noexcept
                    : SubSystemServer("OpenRoaming_GenericRadius", "GENRAD", "genrad") {
            }
        };

    }

    inline auto OpenRoaming_GenericRadius() { return GenericRadius::OpenRoaming::instance(); }
}
