//
// Created by stephane bourque on 2021-09-29.
//

#include "SDK_stubs.h"
#include "Daemon.h"
#include "Poco/Net/HTTPResponse.h"

namespace OpenWifi::SDK {

    bool DeviceSetVenue(const std::string & SerialNumber, const std::string &uuid, Poco::JSON::Object::Ptr & Response) {
        Types::StringPairVec    QueryData;
        Poco::JSON::Object      Body;

        Body.set("serialNumber", SerialNumber);
        Body.set("venue", uuid);
        OpenWifi::OpenAPIRequestPut R(OpenWifi::uSERVICE_GATEWAY,
                                      "/api/v1/device/" +SerialNumber,
                                      QueryData,
                                      Body,
                                      10000);
        if(R.Do(Response) == Poco::Net::HTTPResponse::HTTP_OK) {
            return true;
        } else {
            std::ostringstream os;
            Poco::JSON::Stringifier::stringify(Response,os);
            // std::cout << "Response: " << os.str() << std::endl;
        }

        return false;
    }

    bool SendConfigureCommand(const std::string &SerialNumber, Poco::JSON::Object::Ptr & Configuration, Poco::JSON::Object::Ptr & Response) {

        Types::StringPairVec    QueryData;
        Poco::JSON::Object      Body;

        Poco::JSON::Parser P;
        uint64_t Now = std::time(nullptr);

        Configuration->set("uuid", Now);
        Body.set("serialNumber", SerialNumber);
        Body.set("UUID", Now);
        Body.set("when",0);
        Body.set("configuration", Configuration);

        OpenWifi::OpenAPIRequestPost R(OpenWifi::uSERVICE_GATEWAY,
                                      "/api/v1/device/" + SerialNumber + "/configure",
                                      QueryData,
                                      Body,
                                      10000);

        if(R.Do(Response) == Poco::Net::HTTPResponse::HTTP_OK) {
            return true;
        }

        std::ostringstream os;
        Poco::JSON::Stringifier::stringify(Response,os);

        return false;
    }

    bool GetSubscriberInfo(const std::string &Id, SecurityObjects::UserInfo & User) {
        Types::StringPairVec    QueryData;
        Poco::JSON::Object      Body;

        Poco::JSON::Parser P;
        uint64_t Now = std::time(nullptr);

        OpenWifi::OpenAPIRequestGet TheRequest(OpenWifi::uSERVICE_SECURITY,
                                       "/api/v1/subuser/" + Id + "/configure",
                                       QueryData,
                                       10000);

        Poco::JSON::Object::Ptr TheResponse;
        auto StatusCode = TheRequest.Do(TheResponse,"");
        if(StatusCode == Poco::Net::HTTPResponse::HTTP_OK) {
            try {
                User.from_json(TheResponse);
                return true;
            } catch (...) {

            }
        }

        return false;

    }


}