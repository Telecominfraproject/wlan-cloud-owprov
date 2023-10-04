//
// Created by stephane bourque on 2023-09-11.
//

#include "GlobalReach.h"
#include <Poco/JWT/Token.h>
#include <Poco/JWT/Signer.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/URI.h>
#include <Poco/TemporaryFile.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <framework/RESTAPI_Handler.h>
#include <framework/MicroServiceFuncs.h>
#include <StorageService.h>

namespace OpenWifi {

    namespace GlobalReach {
        int OpenRoaming::Start() {
            poco_information(Logger(), "Starting...");
            InitCache();
            return 0;
        }

        void OpenRoaming::Stop() {
            poco_information(Logger(), "Stopping...");
            poco_information(Logger(), "Stopped...");
        }

        void OpenRoaming::InitCache() {

            auto F = [&](const ProvObjects::GLBLRAccountInfo &Info) {
                poco_information(Logger(), fmt::format("Adding {} to cache.", Info.info.name));
                if (!Info.privateKey.empty() && !Info.GlobalReachAcctId.empty()) {
                    MakeToken(Info.GlobalReachAcctId, Info.privateKey);
                }
                return true;
            };

            StorageService()->GLBLRAccountInfoDB().Iterate(F);
        }

        bool OpenRoaming::Render(const OpenWifi::ProvObjects::RADIUSEndPoint &RE, const std::string &SerialNumber, Poco::JSON::Object &Result) {
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

        bool OpenRoaming::CreateRADSECCertificate(
            const std::string &GlobalReachAccountId,
            const std::string &Name,
            const std::string &CSR,
            ProvObjects::GLBLRCertificateInfo &NewCertificate) {

            try {
                std::cout << __LINE__ << ":" << GlobalReachAccountId << std::endl;
                auto BearerToken = MakeToken(GlobalReachAccountId);
                Poco::URI URI{"https://config.openro.am/v1/radsec/issue"};
                std::string Path(URI.getPathAndQuery());
                Poco::Net::HTTPRequest Request(Poco::Net::HTTPRequest::HTTP_POST, Path,
                                               Poco::Net::HTTPMessage::HTTP_1_1);
                Request.add("Authorization", "Bearer " + BearerToken);

                Poco::Net::HTTPSClientSession Session(URI.getHost(), URI.getPort());
                Session.setTimeout(Poco::Timespan(10000, 10000));
                Poco::JSON::Object CertRequestBody;
                CertRequestBody.set("name", Name);
                CertRequestBody.set("csr", CSR);

                std::ostringstream os;
                CertRequestBody.stringify(os);
                Request.setContentType("application/json");
                Request.setContentLength((long) os.str().size());

                auto &Body = Session.sendRequest(Request);
                Body << os.str();

                Poco::Net::HTTPResponse Response;
                std::istream &is = Session.receiveResponse(Response);
                if (Response.getStatus() == Poco::Net::HTTPResponse::HTTP_OK) {
                    Poco::JSON::Parser P;
                    auto Result = P.parse(is).extract<Poco::JSON::Object::Ptr>();
                    RESTAPIHandler::AssignIfPresent(Result, "certificate", NewCertificate.certificate);
                    RESTAPIHandler::AssignIfPresent(Result, "certificate_chain", NewCertificate.certificateChain);
                    RESTAPIHandler::AssignIfPresent(Result, "certificate_id", NewCertificate.certificateId);
                    RESTAPIHandler::AssignIfPresent(Result, "expires_at", NewCertificate.expiresAt);
                    return true;
                }
                Poco::JSON::Parser P;
                std::ostringstream oos;
                auto Result = P.parse(is).extract<Poco::JSON::Object::Ptr>();
                Result->stringify(oos);
            } catch (const Poco::Exception &E) {
                poco_error(Logger(),
                           fmt::format("Could not create a new RADSEC certificate: {},{}", E.name(), E.displayText()));
            }
            return false;
        }

        bool OpenRoaming::GetRADSECCertificate(
                const std::string &GlobalReachAccountId,
                std::string &CertificateId,
                ProvObjects::GLBLRCertificateInfo &NewCertificate) {

            try {
                Poco::URI URI{fmt::format("https://config.openro.am/v1/radsec/cert/{}", CertificateId)};

                std::string Path(URI.getPathAndQuery());

                Poco::Net::HTTPRequest Request(Poco::Net::HTTPRequest::HTTP_GET, Path,
                                               Poco::Net::HTTPMessage::HTTP_1_1);

                auto BearerToken = MakeToken(GlobalReachAccountId);
                Request.add("Authorization", "Bearer " + BearerToken);

                Poco::Net::HTTPSClientSession Session(URI.getHost(), URI.getPort());
                Session.setTimeout(Poco::Timespan(10000, 10000));

                Session.sendRequest(Request);

                Poco::Net::HTTPResponse Response;
                std::istream &is = Session.receiveResponse(Response);
                if (Response.getStatus() == Poco::Net::HTTPResponse::HTTP_OK) {
                    Poco::JSON::Parser P;
                    auto Result = P.parse(is).extract<Poco::JSON::Object::Ptr>();
                    RESTAPIHandler::AssignIfPresent(Result, "certificate", NewCertificate.certificate);
                    RESTAPIHandler::AssignIfPresent(Result, "certificate_chain", NewCertificate.certificateChain);
                    RESTAPIHandler::AssignIfPresent(Result, "certificate_id", NewCertificate.certificateId);
                    RESTAPIHandler::AssignIfPresent(Result, "expires_at", NewCertificate.expiresAt);
                    return true;
                }
            } catch (const Poco::Exception &E) {
                poco_error(Logger(), fmt::format("Could not retrieve the certificate from GlobalReach: {},{}", E.name(),
                                                 E.displayText()));
            }
            return false;
        }

        std::string
        OpenRoaming::MakeToken(const std::string &GlobalReachAccountId, const std::string &PrivateKey) {
            try {
                Poco::JWT::Token token;
                token.setType("JWT");
                token.setAlgorithm("ES256");
                token.setIssuedAt(std::time(nullptr));

                token.payload().set("iss", GlobalReachAccountId);
                token.payload().set("iat", (unsigned long) std::time(nullptr));

                Poco::SharedPtr<Poco::Crypto::ECKey> Key;
                auto KeyHash = Utils::ComputeHash(PrivateKey);
                auto KeyHint = PrivateKeys_.find(GlobalReachAccountId);
                if (KeyHint != PrivateKeys_.end() && PrivateKey.empty()) {
                    Key = KeyHint->second.second;
                } else {
                    if (PrivateKey.empty()) {
                        return "";
                    }
                    Poco::TemporaryFile F;
                    std::ofstream ofs(F.path().c_str(),
                                      std::ios_base::trunc | std::ios_base::out | std::ios_base::binary);
                    ofs << PrivateKey;
                    ofs.close();
                    auto NewKey = Poco::SharedPtr<Poco::Crypto::ECKey>(
                            new Poco::Crypto::ECKey("", F.path(), ""));
                    Key = NewKey;
                    PrivateKeys_[GlobalReachAccountId] = std::make_pair(KeyHash, NewKey);
                }

                Poco::JWT::Signer Signer;
                Signer.setECKey(Key);
                Signer.addAllAlgorithms();
                return Signer.sign(token, Poco::JWT::Signer::ALGO_ES256);
            } catch (const Poco::Exception &E) {
                poco_error(Logger(),
                           fmt::format("Cannot create a Global Reach token: {},{}", E.name(), E.displayText()));
            }
            return "";
        }

        bool
        OpenRoaming::VerifyAccount(const std::string &GlobalReachAccountId, const std::string &PrivateKey,
                                               std::string &Name) {
            auto BearerToken = MakeToken(GlobalReachAccountId, PrivateKey);

            Poco::URI URI{"https://config.openro.am/v1/config"};
            std::string Path(URI.getPathAndQuery());
            Poco::Net::HTTPRequest Request(Poco::Net::HTTPRequest::HTTP_GET, Path,
                                           Poco::Net::HTTPMessage::HTTP_1_1);
            Request.add("Authorization", "Bearer " + BearerToken);

            Poco::Net::HTTPSClientSession Session(URI.getHost(), URI.getPort());
            Session.setTimeout(Poco::Timespan(10000, 10000));
            Session.sendRequest(Request);
            Poco::Net::HTTPResponse Response;
            std::istream &is = Session.receiveResponse(Response);
            if (Response.getStatus() == Poco::Net::HTTPResponse::HTTP_OK) {
                Poco::JSON::Parser P;
                auto Result = P.parse(is).extract<Poco::JSON::Object::Ptr>();
                if (Result->has("name")) {
                    Name = Result->get("name").toString();
                }
                return true;
            }
            return false;
        }

        std::vector<Utils::HostNameServerResult> OpenRoaming::GetServers() {
            const std::string &domain = "openro.am";
            auto Naptrs = Utils::getNAPTRRecords(domain);
            std::vector<Utils::HostNameServerResult>   Results;

            for(const auto &rec:Naptrs) {
                if(rec.service=="\"aaa+auth:radius.tls.tcp\"") {
                    auto Srvs = Utils::getSRVRecords(rec.replacement);
                    for(const auto &srv:Srvs) {
                        Utils::HostNameServerResult    R{srv.srvname,srv.port};
                        Results.emplace_back(R);
                    }
                }
            }
            return Results;
        }

    }

} // OpenWifi