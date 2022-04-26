//
// Created by stephane bourque on 2021-09-07.
//

#include "APConfig.h"
#include "StorageService.h"

namespace OpenWifi {

    APConfig::APConfig(const std::string &SerialNumber, const std::string &DeviceType, Poco::Logger &L, bool Explain)
        :   SerialNumber_(SerialNumber),
            DeviceType_(DeviceType),
            Logger_(L),
            Explain_(Explain)
    {
    }

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

    static void ShowJSON([[maybe_unused]] const char *S, [[maybe_unused]] const Poco::JSON::Object::Ptr &Obj) {
        /*
        std::stringstream O;
        Poco::JSON::Stringifier::stringify(Obj,O);
        std::cout << S << ":" << std::endl;
        std::cout << ">>>" << std::endl << O.str() << std::endl << "<<<" << std::endl;
         */
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
                        ShowJSON("Data to be merged", B_Radio);
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
        return true;
    }

    bool APConfig::ReplaceVariablesInObject( Poco::JSON::Object::Ptr & Original, Poco::JSON::Object::Ptr & Result) {
        // get all the names and expand
        std::cout << __LINE__ << std::endl;
        auto Names = Original->getNames();
        for(const auto &i:Names) {
            std::cout << __LINE__ << std::endl;
            if(i=="__variableBlock") {
                std::cout << __LINE__ << std::endl;
                if(Original->isArray(i)) {
                    std::cout << __LINE__ << std::endl;
                    auto UUIDs = Original->getArray(i);
                    for(const auto &uuid:*UUIDs) {
                        std::cout << __LINE__ << std::endl;
                        ProvObjects::VariableBlock  VB;
                        if(StorageService()->VariablesDB().GetRecord("id", uuid, VB)) {
                            std::cout << __LINE__ << std::endl;
                            for(const auto &var:VB.variables) {
                                std::cout << __LINE__ << std::endl;
                                Poco::JSON::Parser P;
                                auto VariableBlockInfo = P.parse(var.value).extract<Poco::JSON::Object::Ptr>();
                                auto VarNames = VariableBlockInfo->getNames();
                                for(const auto &j:VarNames) {
                                    std::cout << __LINE__ << std::endl;
                                    Result->set(j,VariableBlockInfo->get(j));
                                }
                            }
                        }
                    }
                }
            } else if(Original->isArray(i)) {
                auto Arr = Poco::makeShared<Poco::JSON::Array>();
                auto Obj = Original->getArray(i);
                ReplaceVariablesInArray(Obj,Arr);
                Result->set(i,Arr);
            } else if (Original->isObject(i)) {
                auto Expanded = Poco::makeShared<Poco::JSON::Object>();
                auto Obj = Original->getObject(i);
                ReplaceVariablesInObject(Obj,Expanded);
                Result->set(i,Expanded);
            } else {
                Result->set(i,Original->get(i));
            }
        }
        return true;
    }

    bool APConfig::ReplaceVariablesInArray( Poco::JSON::Array::Ptr & Original, Poco::JSON::Array::Ptr & ResultArray) {
        for(const auto &element:*Original) {
            if(element.isArray()) {
                auto Expanded = Poco::makeShared<Poco::JSON::Array>();
                auto Object = element.extract<Poco::JSON::Array::Ptr>();
                ReplaceVariablesInArray(Object,Expanded);
                ResultArray->add(Expanded);
            } else if(element.isStruct()) {
                auto Expanded = Poco::makeShared<Poco::JSON::Object>();
                auto Obj = element.extract<Poco::JSON::Object::Ptr>();
                ReplaceVariablesInObject(Obj,Expanded);
                ResultArray->add(Expanded);
            } else {
                ResultArray->add(element);
            }
        }
        return true;
    }

    bool APConfig::Get(Poco::JSON::Object::Ptr & Configuration) {
        if(Config_.empty()) {
            Explanation_.clear();
            try {
                ProvObjects::InventoryTag   D;
                if(StorageService()->InventoryDB().GetRecord("serialNumber", SerialNumber_, D)) {
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

        std::set<std::string>   Sections;
        for(const auto &i:Config_) {
            Poco::JSON::Parser  P;
            auto O = P.parse(i.element.configuration).extract<Poco::JSON::Object::Ptr>();
            auto Names = O->getNames();
            for(const auto &SectionName:Names) {
                auto InsertInfo = Sections.insert(SectionName);
                if (InsertInfo.second) {
                    if (O->isArray(SectionName)) {
                        auto OriginalArray = O->getArray(SectionName);
                        if (Explain_) {
                            Poco::JSON::Object ExObj;
                            ExObj.set("from-uuid", i.info.id);
                            ExObj.set("from-name", i.info.name);
                            ExObj.set("action", "added");
                            ExObj.set("element", OriginalArray);
                            Explanation_.add(ExObj);
                        }
                        auto ExpandedArray = Poco::makeShared<Poco::JSON::Array>();
                        ReplaceVariablesInArray(OriginalArray, ExpandedArray);
                        Configuration->set(SectionName, ExpandedArray);
                    } else if (O->isObject(SectionName)) {
                        auto OriginalSection = O->get(SectionName).extract<Poco::JSON::Object::Ptr>();
                        if (Explain_) {
                            Poco::JSON::Object ExObj;
                            ExObj.set("from-uuid", i.info.id);
                            ExObj.set("from-name", i.info.name);
                            ExObj.set("action", "added");
                            ExObj.set("element", OriginalSection);
                            Explanation_.add(ExObj);
                        }
                        auto ExpandedSection = Poco::makeShared<Poco::JSON::Object>();
                        ReplaceVariablesInObject(OriginalSection, ExpandedSection);
                        Configuration->set(SectionName, ExpandedSection);
                    } else {

                    }
                } else {
                    if (Explain_) {
                        Poco::JSON::Object ExObj;
                        ExObj.set("from-uuid", i.info.id);
                        ExObj.set("from-name", i.info.name);
                        ExObj.set("action", "ignored");
                        ExObj.set("reason", "weight insufficient");
                        ExObj.set("element", O->get(SectionName));
                        Explanation_.add(ExObj);
                    }
                }
            }
        }
        if(Config_.empty())
            return false;

        return true;
    }

    static bool DeviceTypeMatch(const std::string &DeviceType, const Types::StringVec & Types) {
        for(const auto &i:Types) {
            if(i=="*" || Poco::icompare(DeviceType,i)==0)
                return true;
        }
        return false;
    }

    void APConfig::AddConfiguration(const Types::UUIDvec_t &UUIDs) {
        for(const auto &i:UUIDs)
            AddConfiguration(i);
    }

    void APConfig::AddConfiguration(const std::string &UUID) {
        if(UUID.empty())
            return;

        ProvObjects::DeviceConfiguration    Config;
        if(StorageService()->ConfigurationDB().GetRecord("id", UUID, Config)) {
            if(!Config.configuration.empty()) {
                if(DeviceTypeMatch(DeviceType_,Config.deviceTypes)) {
                    for(const auto &i:Config.configuration) {
                        if(i.weight==0) {
                            VerboseElement  VE{ .element = i, .info = Config.info};
                            Config_.push_back(VE);
                        } else {
                            // we need to insert after everything bigger or equal
                            auto Hint = std::lower_bound(Config_.cbegin(),Config_.cend(),i.weight,
                                                         [](const VerboseElement &Elem, uint64_t Value) {
                                return Elem.element.weight>=Value; });
                            VerboseElement  VE{ .element = i, .info = Config.info};
                            Config_.insert(Hint,VE);
                        }
                    }
                } else {
                    Poco::JSON::Object  ExObj;
                    ExObj.set("from-uuid", Config.info.id);
                    ExObj.set("from-name",Config.info.name );
                    ExObj.set("action", "ignored");
                    ExObj.set("reason", "deviceType mismatch");
                    Explanation_.add(ExObj);
                }
            }
        }
    }

    void APConfig::AddEntityConfig(const std::string &UUID) {
        ProvObjects::Entity E;
        if(StorageService()->EntityDB().GetRecord("id",UUID,E)) {
            AddConfiguration(E.configurations);
            if(!E.parent.empty()) {
                AddEntityConfig(E.parent);
            }
        } else {
        }
    }

    void APConfig::AddVenueConfig(const std::string &UUID) {
        ProvObjects::Venue V;
        if(StorageService()->VenueDB().GetRecord("id",UUID,V)) {
            AddConfiguration(V.configurations);
            if(!V.entity.empty()) {
                AddEntityConfig(V.entity);
            } else if(!V.parent.empty()) {
                AddVenueConfig(V.parent);
            }
        } else {
        }
    }
}