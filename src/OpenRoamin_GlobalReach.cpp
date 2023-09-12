//
// Created by stephane bourque on 2023-09-11.
//

#include "OpenRoamin_GlobalReach.h"
#include <Poco/JWT/Token.h>
#include <Poco/JWT/Signer.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/URI.h>
#include <Poco/TemporaryFile.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

#include <framework/MicroServiceFuncs.h>

namespace OpenWifi {

    int OpenRoaming_GlobalReach::Start() {
        poco_information(Logger(), "Starting...");
        return 0;
    }

    void OpenRoaming_GlobalReach::Stop() {
        poco_information(Logger(), "Stopping...");
        poco_information(Logger(), "Stopped...");
    }

    bool OpenRoaming_GlobalReach::GetAccountInfo(
            [[maybe_unused]] const std::string &AccountName,
            [[maybe_unused]] ProvObjects::GLBLRAccountInfo &Account) {
/*        Poco::URI   URI{"https://config.openro.am/v1/config"};

        std::string Path(URI.getPathAndQuery());

        Poco::Net::HTTPRequest Request(Poco::Net::HTTPRequest::HTTP_GET, Path,
                                       Poco::Net::HTTPMessage::HTTP_1_1);

        Request.add("Authorization", "Bearer " + BearerToken);

        Poco::Net::HTTPSClientSession Session(URI.getHost(), URI.getPort());
        Session.setTimeout(Poco::Timespan(10000, 10000));

        Session.sendRequest(Request);

        Poco::Net::HTTPResponse Response;
        std::istream &is = Session.receiveResponse(Response);
        Poco::JSON::Parser P;
        Result= P.parse(is).extract<Poco::JSON::Object::Ptr>();

        std::cout << Response.getStatus() << " : " ;
        Result->stringify(std::cout);
        std::cout << std::endl;
  */
        return true;
    }

    bool OpenRoaming_GlobalReach::CreateRadsecCertificate(
            [[maybe_unused]] const std::string &AccountName,
            [[maybe_unused]] ProvObjects::GLBLRCertificateInfo &NewCertificate) {
/*
        Poco::URI   URI{"https://config.openro.am/v1/radsec/issue"};

        std::string Path(URI.getPathAndQuery());

        Poco::Net::HTTPRequest Request(Poco::Net::HTTPRequest::HTTP_POST, Path,
                                       Poco::Net::HTTPMessage::HTTP_1_1);

        Request.add("Authorization", "Bearer " + BearerToken);

        Poco::Net::HTTPSClientSession Session(URI.getHost(), URI.getPort());
        Session.setTimeout(Poco::Timespan(10000, 10000));

        std::ostringstream os;
        Body.stringify(os);
        Request.setContentType("application/json");
        Request.setContentLength(os.str().size());

        auto &body = Session.sendRequest(Request);
        body << os.str();

        Poco::Net::HTTPResponse Response;
        std::istream &is = Session.receiveResponse(Response);
        Poco::JSON::Parser P;
        Result= P.parse(is).extract<Poco::JSON::Object::Ptr>();

        std::cout << Response.getStatus() << " : " ;
        Result->stringify(std::cout);
        std::cout << std::endl;
*/
        return true;
    }

    bool OpenRoaming_GlobalReach::GetRadsecCertificate(
        [[maybe_unused]] const std::string &AccountName,
        [[maybe_unused]] std::string &CertificateId,
        [[maybe_unused]] ProvObjects::GLBLRCertificateInfo &NewCertificate) {
        return true;
    }

    std::string OpenRoaming_GlobalReach::MakeToken(const std::string &GlobalReachAccountId, const std::string &PrivateKey) {
        Poco::JWT::Token token;

        token.setType("JWT");
        token.setAlgorithm("ES256");
        token.setIssuedAt(std::time(nullptr));

        token.payload().set("iss", GlobalReachAccountId);
        token.payload().set("iat", (unsigned long) std::time(nullptr));

        Poco::SharedPtr<Poco::Crypto::ECKey>    Key;
        auto KeyHash = Utils::ComputeHash(PrivateKey);
        auto KeyHint = PrivateKeys_.find(KeyHash);
        if(KeyHint!=PrivateKeys_.end()) {
            Key = KeyHint->second;
        } else {
            Poco::TemporaryFile F;
            std::ofstream ofs(F.path().c_str(),std::ios_base::trunc|std::ios_base::out|std::ios_base::binary);
            ofs << PrivateKey;
            ofs.close();
            auto NewKey = Poco::SharedPtr<Poco::Crypto::ECKey>(
                    new Poco::Crypto::ECKey("", F.path(),""));
            Key = PrivateKeys_[KeyHash] = NewKey;
        }

        Poco::JWT::Signer Signer;
        Signer.setECKey(Key);
        Signer.addAllAlgorithms();
        return Signer.sign(token, Poco::JWT::Signer::ALGO_ES256);
    }

    bool OpenRoaming_GlobalReach::VerifyAccount(const std::string &GlobalReachAccountId, const std::string &PrivateKey, [[
    maybe_unused]] std::string &Name) {
        auto BearerToken = MakeToken(GlobalReachAccountId, PrivateKey);

        Poco::URI   URI{"https://config.openro.am/v1/config"};
        std::string Path(URI.getPathAndQuery());
        Poco::Net::HTTPRequest Request(Poco::Net::HTTPRequest::HTTP_GET, Path,
                                       Poco::Net::HTTPMessage::HTTP_1_1);
        Request.add("Authorization", "Bearer " + BearerToken);

        Poco::Net::HTTPSClientSession Session(URI.getHost(), URI.getPort());
        Session.setTimeout(Poco::Timespan(10000, 10000));
        Session.sendRequest(Request);
        Poco::Net::HTTPResponse Response;
        std::istream &is = Session.receiveResponse(Response);
        if(Response.getStatus()==Poco::Net::HTTPResponse::HTTP_OK) {
            Poco::JSON::Parser P;
            auto Result = P.parse(is).extract<Poco::JSON::Object::Ptr>();
            if(Result->has("name")) {
                Name = Result->get("name").toString();
            }
            return true;
        }
        return false;
    }


} // OpenWifi