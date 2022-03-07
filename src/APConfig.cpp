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
        _OWDEBUG_
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

    static void ShowJSON(const char *S, const Poco::JSON::Object::Ptr &Obj) {
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

    void APConfig::AddVariables(const Poco::JSON::Object::Ptr &Section, Poco::JSON::Object::Ptr &Result) {
        auto Names = Section->getNames();
        for(const auto &v:Names) {
            _OWDEBUG_
            if(v=="__variableBlock") {
                //  process the variable
                _OWDEBUG_
                auto uuids = Section->get(v);
                _OWDEBUG_
                if(uuids.isArray()) {
                    VariablesDB::RecordName VarInfo;
                    _OWDEBUG_
                    if (StorageService()->VariablesDB().GetRecord("id", uuids, VarInfo)) {
                        _OWDEBUG_
                        Poco::JSON::Parser P;
                        for (const auto &var: VarInfo.variables) {
                            _OWDEBUG_
                            auto vv = P.parse(var.value).extract<Poco::JSON::Object::Ptr>();
                            auto VarNames = vv->getNames();
                            for (const auto &single_var: VarNames)
                                Result->set(single_var, vv->get(single_var));
                        }
                    } else {
                        _OWDEBUG_
                        Result->set(v, Section->get(v));
                    }
                }
            } else {
                _OWDEBUG_
                Result->set(v,Section->get(v));
            }
        }
    }

    bool APConfig::Get(Poco::JSON::Object::Ptr & Configuration) {
        _OWDEBUG_
        if(Config_.empty()) {
            _OWDEBUG_
            Explanation_.clear();
            _OWDEBUG_
            try {
                ProvObjects::InventoryTag   D;
                _OWDEBUG_
                if(StorageService()->InventoryDB().GetRecord("serialNumber", SerialNumber_, D)) {
                    _OWDEBUG_
                    if(!D.deviceConfiguration.empty()) {
                        _OWDEBUG_
                        AddConfiguration(D.deviceConfiguration);
                    }
                    if(!D.entity.empty()) {
                        _OWDEBUG_
                        AddEntityConfig(D.entity);
                    } else if(!D.venue.empty()) {
                        _OWDEBUG_
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
        // auto Tmp=Poco::makeShared<Poco::JSON::Object>();
        _OWDEBUG_
        std::set<std::string>   Sections;
        _OWDEBUG_
        for(const auto &i:Config_) {
            _OWDEBUG_
            ShowJSON("Iteration Start:", Configuration);
            _OWDEBUG_
            Poco::JSON::Parser  P;
            auto O = P.parse(i.element.configuration).extract<Poco::JSON::Object::Ptr>();
            _OWDEBUG_
            auto Names = O->getNames();
            _OWDEBUG_
            auto SectionName = Names[0];
            std::cout << "Names[0]" << SectionName << std::endl;
            _OWDEBUG_
            if(O->isArray(SectionName)) {
                _OWDEBUG_
                Configuration->set(SectionName, O->get(SectionName));
                _OWDEBUG_
            } else {
                auto SectionInfo = O->getObject(SectionName);
                _OWDEBUG_
                auto InsertInfo = Sections.insert(SectionName);
                _OWDEBUG_
                if (InsertInfo.second) {
                    if (Explain_) {
                        Poco::JSON::Object ExObj;
                        ExObj.set("from-uuid", i.info.id);
                        ExObj.set("from-name", i.info.name);
                        ExObj.set("action", "added");
                        ExObj.set("element", SectionInfo);
                        Explanation_.add(ExObj);
                    }
                    _OWDEBUG_
                    auto Result = Poco::makeShared<Poco::JSON::Object>();
                    _OWDEBUG_
                    AddVariables(SectionInfo, Result);
                    _OWDEBUG_
                    Configuration->set(SectionName, Result);
                    _OWDEBUG_
                } else {
                    if (Explain_) {
                        Poco::JSON::Object ExObj;
                        ExObj.set("from-uuid", i.info.id);
                        ExObj.set("from-name", i.info.name);
                        ExObj.set("action", "ignored");
                        ExObj.set("reason", "weight insufficient");
                        ExObj.set("element", SectionInfo);
                        Explanation_.add(ExObj);
                    }
                }
            }
        }
        // Configuration = Tmp;
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
                                                         [](const VerboseElement &Elem, int Value) {
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
        _OWDEBUG_
        if(StorageService()->EntityDB().GetRecord("id",UUID,E)) {
            _OWDEBUG_
            AddConfiguration(E.configurations);
            _OWDEBUG_
            if(!E.parent.empty()) {
                _OWDEBUG_
                AddEntityConfig(E.parent);
            }
        } else {
            _OWDEBUG_
        }
    }

    void APConfig::AddVenueConfig(const std::string &UUID) {
        ProvObjects::Venue V;
        _OWDEBUG_
        if(StorageService()->VenueDB().GetRecord("id",UUID,V)) {
            _OWDEBUG_
            AddConfiguration(V.configurations);
            _OWDEBUG_
            if(!V.entity.empty()) {
                _OWDEBUG_
                AddEntityConfig(V.entity);
            } else if(!V.parent.empty()) {
                _OWDEBUG_
                AddVenueConfig(V.parent);
            }
        } else {
            _OWDEBUG_
        }
    }
}