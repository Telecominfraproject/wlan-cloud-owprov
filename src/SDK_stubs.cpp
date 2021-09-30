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
            std::cout << "Response: " << os.str() << std::endl;
        }

        return false;
    }

}