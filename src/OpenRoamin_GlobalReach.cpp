//
// Created by stephane bourque on 2023-09-11.
//

#include "OpenRoamin_GlobalReach.h"

namespace OpenWifi {

    int OpenRoaming_GlobalReach::Start() {
        poco_information(Logger(), "Starting...");
        return 0;
    }

    void OpenRoaming_GlobalReach::Stop() {
        poco_information(Logger(), "Stopping...");
        poco_information(Logger(), "Stopped...");
    }

    bool OpenRoaming_GlobalReach::GetAccountInfo(const std::string &AccountName, ProvObjects::GLBLRAccountInfo &Account) {
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

    bool OpenRoaming_GlobalReach::CreateRadsecCertificate(const std::string &AccountName, ProvObjects::GLBLRCertificateInfo &NewCertificate) {
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

    bool OpenRoaming_GlobalReach::GetRadsecCertificate(const std::string &AccountName, std::string &CertificateId,
                                                       ProvObjects::GLBLRCertificateInfo &NewCertificate) {
        return true;
    }

} // OpenWifi