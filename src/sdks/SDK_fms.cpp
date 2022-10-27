//
// Created by stephane bourque on 2022-05-12.
//

#include "SDK_fms.h"

#include "RESTObjects/RESTAPI_FMSObjects.h"

#include "framework/MicroServiceNames.h"
#include "framework/OpenAPIRequests.h"

namespace OpenWifi::SDK::FMS {

    namespace Firmware {

        bool GetLatest(const std::string &device_type, bool RCOnly, FMSObjects::Firmware & FirmWare) {
            static const std::string EndPoint{"/api/v1/firmwares"};

            OpenWifi::OpenAPIRequestGet     API( uSERVICE_FIRMWARE,
                                            EndPoint,
                                                 { { "latestOnly" , "true"},
                                                   { "deviceType", device_type},
                                                   { "rcOnly" , RCOnly ? "true" : "false" }
                                                   },
                                                 50000);

            auto CallResponse = Poco::makeShared<Poco::JSON::Object>();
            auto StatusCode = API.Do(CallResponse);
            if( StatusCode == Poco::Net::HTTPResponse::HTTP_OK) {
                return FirmWare.from_json(CallResponse);
            }
            return false;
        }

    }

};
