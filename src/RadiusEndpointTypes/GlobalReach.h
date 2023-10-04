//
// Created by stephane bourque on 2023-09-11.
//

#pragma once

#include "framework/SubSystemServer.h"
#include "framework/utils.h"
#include "Poco/JSON/Object.h"
#include "RESTObjects/RESTAPI_ProvObjects.h"

namespace OpenWifi {

    namespace GlobalReach {
        class OpenRoaming : public SubSystemServer {
        public:
            static auto instance() {
                static auto instance_ = new OpenRoaming;
                return instance_;
            }

            int Start() override;

            void Stop() override;

            bool CreateRADSECCertificate(const std::string &AccountName,
                                         const std::string &Name,
                                         const std::string &CSR,
                                         ProvObjects::GLBLRCertificateInfo &NewCertificate);

            bool GetRADSECCertificate(const std::string &AccountName, std::string &CertificateId,
                                      ProvObjects::GLBLRCertificateInfo &NewCertificate);

            bool
            VerifyAccount(const std::string &GlobalReachAccountId, const std::string &PrivateKey, std::string &Name);

            void InitCache();

            bool Render(const OpenWifi::ProvObjects::RADIUSEndPoint &RE, const std::string & SerialNUmber, Poco::JSON::Object &Result);

        private:
            std::string MakeToken(const std::string &GlobalReachAccountId, const std::string &PrivateKey = "");

            std::map<std::string, std::pair<std::string, Poco::SharedPtr<Poco::Crypto::ECKey>>> PrivateKeys_;

            OpenRoaming() noexcept
                    : SubSystemServer("OpenRoaming_GlobalReach", "GLBL-REACH", "globalreach") {
            }
        };

        std::vector<Utils::HostNameServerResult> GetServers();
    }

    inline auto OpenRoaming_GlobalReach() { return GlobalReach::OpenRoaming::instance(); }

} // OpenWifi

