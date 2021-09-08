//
// Created by stephane bourque on 2021-09-07.
//

#include "APConfig.h"
#include "StorageService.h"
#include "APConfig.h"

namespace OpenWifi {

    APConfig::APConfig(const std::string &SerialNumber, const std::string &DeviceType, Poco::Logger &L)
        :   SerialNumber_(SerialNumber),
            DeviceType_(DeviceType),
            Logger_(L)
    {}

    bool APConfig::FindRadio(const std::string &Band, const Poco::JSON::Array::Ptr &Arr, Poco::JSON::Object::Ptr & Radio) {
        for(const auto &i:*Arr) {
            auto R = i.extract<Poco::JSON::Object::Ptr>();
            if(R->has("band") && R->get("band").toString()==Band) {
                Radio = R;
                return true;
            }
        }
        return false;
    }

    bool APConfig::mergeArray(const std::string &K, const Poco::JSON::Array::Ptr &A , const Poco::JSON::Array::Ptr &B, Poco::JSON::Array &Arr) {
        if(K=="radios") {
            int index=0;
            for(const auto &i:*A) {
                auto A_Radio = A->get(index).extract<Poco::JSON::Object::Ptr>();
                if(A_Radio->has("band")) {
                    std::string Band = A_Radio->get("band").toString();
                    Poco::JSON::Object::Ptr B_Radio;
                    if(FindRadio(Band,B,B_Radio)) {
                        Poco::JSON::Object RR;
                        merge(A_Radio,B_Radio,RR);
                        Arr.set(index, RR);
                    } else {
                        Arr.set(index,A);
                    }
                }
                ++index;
            }
        } else {
            Arr = *A;
        }
        return true;
    }

    bool APConfig::merge(const Poco::JSON::Object::Ptr & A, const Poco::JSON::Object::Ptr & B, Poco::JSON::Object &C) {

        for(const auto &i:*A) {
            const std::string & K = i.first;
            //        std::cout << "KEY: " << K << std::endl;
            if(B->has(K)) {
                if(A->isArray(K)) {
                    //                std::cout << "ISARRAY" << std::endl;
                    if(B->isArray(K)) {
                        Poco::JSON::Array   Arr;
                        auto AR1=A->getArray(K);
                        auto AR2=B->getArray(K);
                        mergeArray(K,AR1,AR2,Arr);
                        C.set(K,Arr);
                    } else {
                        C.set(K,A->getArray(K));
                    }
                }
                else if(A->isObject(K) && B->isObject(K)) {
                    //                std::cout << "ISOBJECT" << std::endl;
                    Poco::JSON::Object R;
                    merge(A->getObject(K),B->getObject(K),R);
                    C.set(K,R);
                }
                else {
                    C.set(K,i.second);
                }
            } else {
                C.set(K,i.second);
            }
        }

        for(const auto &i:*B) {
            const std::string & K = i.first;
            if(!A->has(K))
                C.set(K,i.second);
        }

        return true;
    }


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
        //      globals
        //      unit

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

        std::stringstream  O;
        Poco::JSON::Stringifier::stringify(CFG,O);

        Config = O.str();

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