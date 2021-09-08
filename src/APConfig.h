//
// Created by stephane bourque on 2021-09-07.
//

#ifndef OWPROV_APCONFIG_H
#define OWPROV_APCONFIG_H

#include <string>
#include "Poco/Logger.h"
#include "RESTAPI_ProvObjects.h"

namespace OpenWifi {

    typedef std::vector<ProvObjects::DeviceConfiguration> ConfigVec;

    class APConfig {
        public:
            explicit APConfig(const std::string & SerialNumber, const std::string & DeviceType, Poco::Logger & L);


            [[nodiscard]] bool Get(std::string &Config);

            void AddConfiguration(const std::string &UUID);
            void AddVenueConfig(const std::string &UUID);
            void AddEntityConfig(const std::string &UUID);

        private:
            std::string                 SerialNumber_;
            std::string                 DeviceType_;
            Poco::Logger                & Logger_;
            std::string                 CompleteConfig_;
            ConfigVec                   Config_;
            Types::StringPairVec        Errors;
    };
}


#endif //OWPROV_APCONFIG_H
