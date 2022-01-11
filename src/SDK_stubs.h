//
// Created by stephane bourque on 2021-09-29.
//

#pragma once

#include "framework/MicroService.h"

namespace OpenWifi::SDK {

    bool DeviceSetVenue(const std::string & SerialNumber, const std::string &uuid, Poco::JSON::Object::Ptr & Response);
    bool SendConfigureCommand(const std::string &serialNumber, Poco::JSON::Object::Ptr &Configuration, Poco::JSON::Object::Ptr & Response);
    bool GetSubscriberInfo(const std::string &Id, SecurityObjects::UserInfo & User);

};

