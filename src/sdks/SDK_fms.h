//
// Created by stephane bourque on 2022-05-12.
//

#pragma once

#include <string>
#include "RESTObjects/RESTAPI_FMSObjects.h"

namespace OpenWifi::SDK::FMS {

    namespace Firmware {
        bool GetLatest(const std::string &device_type, bool RCOnly, FMSObjects::Firmware & FirmWare);
    }

};


