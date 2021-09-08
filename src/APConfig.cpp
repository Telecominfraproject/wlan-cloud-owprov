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

    bool APConfig::RemoveBand(const std::string &Band, const Poco::JSON::Array::Ptr &A_in,Poco::JSON::Array::Ptr &A_Out) {
        for(const auto &i:*A_in) {
            auto R = i.extract<Poco::JSON::Object::Ptr>();
            if(R->has("band") && R->get("band").toString()==Band) {
            } else {
                A_Out->add(i);
            }
        }
        return false;
    }



    static void ShowJSON(const char *S, const Poco::JSON::Object::Ptr &Obj) {
        std::stringstream O;
        Poco::JSON::Stringifier::stringify(Obj,O);
        std::cout << S << ":" << std::endl;
        std::cout << ">>>" << std::endl << O.str() << std::endl << "<<<" << std::endl;
    }

    bool APConfig::mergeArray(const std::string &K, const Poco::JSON::Array::Ptr &A , const Poco::JSON::Array::Ptr &B, Poco::JSON::Array &Arr) {
        if(K=="radios") {
            auto BB=Poco::makeShared<Poco::JSON::Array>();
            BB = B;
            for(const auto &i:*A) {
                auto A_Radio = i.extract<Poco::JSON::Object::Ptr>();
                // std::cout << "Radio A:" << std::endl;
                // ShowJSON(A_Radio);
                if(A_Radio->has("band")) {
                    std::string Band = A_Radio->get("band").toString();
                    // std::cout << "Looking for band: " << Band << std::endl;
                    auto B_Radio=Poco::makeShared<Poco::JSON::Object>();
                    if(FindRadio(Band,B,B_Radio)) {
                        // std::cout << "Radio B:" << std::endl;
                        // ShowJSON(B_Radio);
                        auto RR = Poco::makeShared<Poco::JSON::Object>();
                        merge(A_Radio, B_Radio,RR);
                        ShowJSON("Merged data", RR);
                        auto CC = Poco::makeShared<Poco::JSON::Array>();
                        RemoveBand(Band, BB, CC );
                        BB = CC;
                        Arr.add(RR);
                    } else {
                        Arr.add(A_Radio);
                    }
                }
            }
            for(const auto &i:*BB)
                Arr.add(i);
        } else {
            Arr = *A;
        }
        return true;
    }

    bool APConfig::merge(const Poco::JSON::Object::Ptr & A, const Poco::JSON::Object::Ptr & B, Poco::JSON::Object::Ptr &C) {
        for(const auto &i:*A) {
            const std::string & K = i.first;
            //  std::cout << "KEY: " << K << std::endl;
            if(B->has(K)) {
                if(A->isArray(K)) {
                    //  std::cout << "ISARRAY" << std::endl;
                    if(B->isArray(K)) {
                        Poco::JSON::Array   Arr;
                        auto AR1=A->getArray(K);
                        auto AR2=B->getArray(K);
                        mergeArray(K,AR1,AR2,Arr);
                        C->set(K,Arr);
                    } else {
                        C->set(K,A->getArray(K));
                    }
                }
                else if(A->isObject(K) && B->isObject(K)) {
                    //  std::cout << "ISOBJECT" << std::endl;
                    auto R=Poco::makeShared<Poco::JSON::Object>();
                    merge(A->getObject(K),B->getObject(K),R);
                    C->set(K,R);
                }
                else {
                    C->set(K,i.second);
                }
            } else {
                C->set(K,i.second);
            }
        }

        /*
        for(const auto &i:*B) {
            const std::string & K = i.first;
            if(!A->has(K)) {
                // std::cout << "Before leave" << std::endl;
                // ShowJSON(C);
                C->set(K, i.second);
                // std::cout << "After leave" << std::endl;
                // ShowJSON(C);
            }
        }
         */

        return true;
    }

    bool APConfig::Get(Poco::JSON::Object::Ptr &Configuration) {
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
        auto Tmp=Poco::makeShared<Poco::JSON::Object>();
        for(const auto &i:Config_) {
            ShowJSON("Iteration Start:", Tmp);
            Poco::JSON::Parser  P;
            auto O = P.parse(i.configuration).extract<Poco::JSON::Object::Ptr>();
            auto Result = Poco::makeShared<Poco::JSON::Object>();
            ShowJSON("O", O);
            ShowJSON("Tmp", Tmp);
            merge(O, Tmp, Result);
            ShowJSON("Iteration End:", Result);
            Tmp = Result;
        }
        Configuration = Tmp;

        if(Config_.empty())
            return false;

        return true;
    }

    void APConfig::AddConfiguration(const std::string &UUID) {
        ProvObjects::DeviceConfiguration    Config;

        if(UUID.empty())
            return;

        if(Storage()->ConfigurationDB().GetRecord("id", UUID,Config)) {
            //  find where to insert into this list using the weight.
            if(!Config.configuration.empty()) {
                for(const auto &i:Config.configuration) {
                    if(i.weight==0) {
                        Config_.push_back(i);
                    } else {
                        // we need to insert after everything bigger or equal
                        auto Hint = std::lower_bound(Config_.cbegin(),Config_.cend(),i.weight,
                                                     [](const ProvObjects::DeviceConfigurationElement &Elem, int Value) {
                                                            return Elem.weight>=Value; });
                        Config_.insert(Hint,i);
                    }
                }
            }
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