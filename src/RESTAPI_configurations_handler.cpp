//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "RESTAPI_configurations_handler.h"
#include "RESTAPI_ProvObjects.h"
#include "StorageService.h"
#include "Daemon.h"
#include "RESTAPI_errors.h"

namespace OpenWifi{

    void RESTAPI_configurations_handler::DoGet() {
        auto UUID = GetBinding("uuid","");
        if(UUID.empty()) {
            BadRequest(RESTAPI::Errors::MissingUUID);
            return;
        }

        ProvObjects::DeviceConfiguration    C;
        if(Storage()->ConfigurationDB().GetRecord("id", UUID, C)) {
            Poco::JSON::Object  Answer;

            C.to_json(Answer);
            ReturnObject(Answer);
            return;
        }
        NotFound();
    }

    void RESTAPI_configurations_handler::DoDelete() {
        auto UUID = GetBinding("uuid","");
        if(UUID.empty()) {
            BadRequest(RESTAPI::Errors::MissingUUID);
            return;
        }

        ProvObjects::DeviceConfiguration    C;
        if(Storage()->ConfigurationDB().GetRecord("id", UUID, C)) {
            if(!C.inUse.empty()) {
                BadRequest(RESTAPI::Errors::StillInUse);
                return;
            }

            if(Storage()->ConfigurationDB().DeleteRecord("id", UUID)) {
                OK();
            } else {
                BadRequest(RESTAPI::Errors::CouldNotBeDeleted);
            }
        } else {
            NotFound();
        }
    }

    //      interfaces
    //      metrics
    //      radios
    //      services
    //      globals
    //      unit
    bool RESTAPI_configurations_handler::ValidateConfigBlock(const ProvObjects::DeviceConfiguration &Config) {
        static const std::vector<std::string> SectionNames{ "globals", "interfaces", "metrics", "radios", "services", "unit" };

        for(const auto &i:Config.configuration) {
            Poco::JSON::Parser  P;
            if(i.name.empty()) {
                BadRequest(RESTAPI::Errors::NameMustBeSet);
                return false;
            }
            auto Blocks = P.parse(i.configuration).extract<Poco::JSON::Object::Ptr>();
            auto N = Blocks->getNames();
            for(const auto &j:N) {
                if(std::find(SectionNames.cbegin(),SectionNames.cend(),j)==SectionNames.cend()) {
                    BadRequest(RESTAPI::Errors::ConfigBlockInvalid);
                    return false;
                }
            }
        }
        return true;
    }

    void RESTAPI_configurations_handler::DoPost() {
        auto UUID = GetBinding("uuid","");
        if(UUID.empty()) {
            BadRequest(RESTAPI::Errors::MissingUUID);
            return;
        }

        ProvObjects::DeviceConfiguration C;
        Poco::JSON::Object::Ptr Obj = ParseStream();
        if (!C.from_json(Obj)) {
            BadRequest(RESTAPI::Errors::InvalidJSONDocument);
            return;
        }

        if(C.info.name.empty()) {
            BadRequest(RESTAPI::Errors::NameMustBeSet);
            return;
        }

        if(!C.managementPolicy.empty() && !Storage()->PolicyDB().Exists("id",C.managementPolicy)) {
            BadRequest(RESTAPI::Errors::UnknownId);
            return;
        }

        C.info.modified = C.info.created = std::time(nullptr);
        C.info.id = Daemon()->CreateUUID();
        for(auto &i:C.info.notes)
            i.createdBy = UserInfo_.userinfo.email;

        C.inUse.clear();
        if(C.deviceTypes.empty() || !Storage()->AreAcceptableDeviceTypes(C.deviceTypes, true)) {
            BadRequest(RESTAPI::Errors::InvalidDeviceTypes);
            return;
        }

        if(!ValidateConfigBlock(C))
            return;

        if(Storage()->ConfigurationDB().CreateRecord(C)) {
            Storage()->ConfigurationDB().GetRecord("id", C.info.id, C);
            Poco::JSON::Object  Answer;

            if(!C.managementPolicy.empty())
                Storage()->PolicyDB().AddInUse("id",C.managementPolicy,Storage()->PolicyDB().Prefix(), C.info.id);

            C.to_json(Answer);
            ReturnObject(Answer);
        } else {
            BadRequest(RESTAPI::Errors::RecordNotCreated);
        }

    }

    void RESTAPI_configurations_handler::DoPut() {
        auto UUID = GetBinding("uuid","");
        if(UUID.empty()) {
            BadRequest(RESTAPI::Errors::MissingUUID);
            return;
        }

        ProvObjects::DeviceConfiguration    Existing;
        if(!Storage()->ConfigurationDB().GetRecord("id", UUID, Existing)) {
            NotFound();
            return;
        }

        ProvObjects::DeviceConfiguration    NewConfig;
        auto Obj = ParseStream();
        if (!NewConfig.from_json(Obj)) {
            BadRequest(RESTAPI::Errors::InvalidJSONDocument);
            return;
        }

        for(auto &i:NewConfig.info.notes) {
            i.createdBy = UserInfo_.userinfo.email;
            Existing.info.notes.insert(Existing.info.notes.begin(),i);
        }

        if(!NewConfig.managementPolicy.empty() && (NewConfig.managementPolicy!=Existing.managementPolicy && !Storage()->PolicyDB().Exists("id",NewConfig.managementPolicy))) {
            BadRequest(RESTAPI::Errors::UnknownManagementPolicyUUID);
            return;
        }

        if(!NewConfig.deviceTypes.empty() && !Storage()->AreAcceptableDeviceTypes(NewConfig.deviceTypes, true)) {
            BadRequest(RESTAPI::Errors::InvalidDeviceTypes);
            return;
        }

        if(!NewConfig.deviceTypes.empty())
            Existing.deviceTypes = NewConfig.deviceTypes;

        if(!NewConfig.info.name.empty())
            Existing.info.name = NewConfig.info.name;

        if(!NewConfig.info.description.empty())
            Existing.info.description = NewConfig.info.description;

        NewConfig.info.modified = std::time(nullptr);

        if(!ValidateConfigBlock(NewConfig))
            return;

        if(!NewConfig.variables.empty())
            Existing.variables = NewConfig.variables;

        std::string OldPolicy;
        OldPolicy = Existing.managementPolicy;
        if(!NewConfig.managementPolicy.empty() && Existing.managementPolicy!=NewConfig.managementPolicy) {
            OldPolicy = Existing.managementPolicy;
            Existing.managementPolicy = NewConfig.managementPolicy;
        }

        if(Storage()->ConfigurationDB().UpdateRecord("id",UUID,Existing)) {
            if(!OldPolicy.empty()) {
                Storage()->PolicyDB().DeleteInUse("id",OldPolicy,Storage()->ConfigurationDB().Prefix(),Existing.info.id);
            }
            if(!Existing.managementPolicy.empty()) {
                Storage()->PolicyDB().AddInUse("id",Existing.managementPolicy,Storage()->ConfigurationDB().Prefix(),Existing.info.id);
            }

            ProvObjects::DeviceConfiguration    D;
            Storage()->ConfigurationDB().GetRecord("id",UUID,D);
            Poco::JSON::Object  Answer;
            D.to_json(Answer);
            ReturnObject(Answer);
            return;
        } else {
            BadRequest(RESTAPI::Errors::RecordNotUpdated);
        }
    }
}