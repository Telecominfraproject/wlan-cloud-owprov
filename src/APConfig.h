//
// Created by stephane bourque on 2021-09-07.
//

#ifndef OWPROV_APCONFIG_H
#define OWPROV_APCONFIG_H

#include <string>
#include "Poco/Logger.h"
#include "RESTAPI_ProvObjects.h"

namespace OpenWifi {

    typedef std::vector<ProvObjects::DeviceConfigurationElement> ConfigVec;

    class APConfig {
        public:
            explicit APConfig(const std::string & SerialNumber, const std::string & DeviceType, Poco::Logger & L);


            [[nodiscard]] bool Get(Poco::JSON::Object &Configuration);

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

            bool FindRadio(const std::string &Band, const Poco::JSON::Array::Ptr &Arr, Poco::JSON::Object::Ptr & Radio);
            bool mergeArray(const std::string &K, const Poco::JSON::Array::Ptr &A , const Poco::JSON::Array::Ptr &B, Poco::JSON::Array &Arr);
            bool merge(const Poco::JSON::Object::Ptr & A, const Poco::JSON::Object::Ptr & B, Poco::JSON::Object &C);

    };
}


#endif //OWPROV_APCONFIG_H
