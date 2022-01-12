//
// Created by stephane bourque on 2022-01-11.
//

#pragma once

#include "RESTObjects/RESTAPI_GWobjects.h"

namespace OpenWifi::SDK::GW {
    namespace Device {
        void Reboot(RESTAPIHandler *client, const std::string & Mac, uint64_t When);
        void LEDs(RESTAPIHandler *client, const std::string & Mac, uint64_t When, uint64_t Duration, const std::string & Pattern);
        void Factory(RESTAPIHandler *client, const std::string & Mac, uint64_t When, bool KeepRedirector);
        void Upgrade(RESTAPIHandler *client, const std::string & Mac, uint64_t When, const std::string & ImageName, bool KeepRedirector);
        void Refresh(RESTAPIHandler *client, const std::string & Mac, uint64_t When);
        void PerformCommand(RESTAPIHandler *client, const std::string &Command, const std::string & EndPoint, Poco::JSON::Object & CommandRequest);
        bool Configure(RESTAPIHandler *client, const std::string &Mac, Poco::JSON::Object::Ptr & Configuration, Poco::JSON::Object::Ptr & Response);

        bool SetVenue(RESTAPIHandler *client, const std::string & SerialNumber, const std::string &uuid);

   }
}

