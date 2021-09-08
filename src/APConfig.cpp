//
// Created by stephane bourque on 2021-09-07.
//

#include "APConfig.h"
#include "StorageService.h"

namespace OpenWifi {

    APConfig::APConfig(const std::string &SerialNumber, const std::string &DeviceType, Poco::Logger &L)
        :   SerialNumber_(SerialNumber),
            DeviceType_(DeviceType),
            Logger_(L)
    {}

    bool APConfig::Get(std::string &Config) {

        if(Config_.empty()) {
            try {
                ProvObjects::InventoryTag   D;
                if(Storage()->InventoryDB().GetRecord("serialNumber", SerialNumber_, D)) {

                    if(!D.deviceConfiguration.empty()) {
                        AddConfiguration(D.deviceConfiguration);
                    }
                    if(!D.entity.empty()) {
                        AddEntityConfig(D.entity);
                    } else if(!D.venue.empty()) {
                        AddVenueConfig(D.venue);
                    }
                }

                //  Now we have all the config we need.
            } catch (const Poco::Exception &E ) {
                Logger_.log(E);
            }
        }

        //  So we have sections...
        //      interfaces
        //      metrics
        //      radios
        //      services
        //

        Poco::JSON::Object  CFG;
        for(const auto &i:Config_) {
            for(const auto &ConfigElement:i.configuration) {
                Poco::JSON::Parser  P;
                auto O = P.parse(ConfigElement.configuration).extract<Poco::JSON::Object::Ptr>();
                for(const auto &j:*O) {
                    CFG.set(j.first,j.second);
                }
            }
        }

        if(Config_.empty())
            return false;

        return true;
    }

    void APConfig::AddConfiguration(const std::string &UUID) {
        ProvObjects::DeviceConfiguration    Config;

        if(UUID.empty())
            return;

        if(Storage()->ConfigurationDB().GetRecord("id", UUID,Config)) {
            Config_.push_back(Config);
        }
    }

    void APConfig::AddEntityConfig(const std::string &UUID) {
        ProvObjects::Entity E;
        if(Storage()->EntityDB().GetRecord("id",UUID,E)) {
            AddConfiguration(E.deviceConfiguration);
            if(!E.parent.empty())
                AddEntityConfig(E.parent);
        }
    }

    void APConfig::AddVenueConfig(const std::string &UUID) {
        ProvObjects::Venue V;
        if(Storage()->VenueDB().GetRecord("id",UUID,V)) {
            AddConfiguration(V.deviceConfiguration);
            if(!V.entity.empty()) {
                AddEntityConfig(V.entity);
            } else if(!V.parent.empty()) {
                AddVenueConfig(V.parent);
            }
        }
    }
}