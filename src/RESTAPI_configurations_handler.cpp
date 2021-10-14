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

#include "ConfigurationValidator.h"

namespace OpenWifi{

    void RESTAPI_configurations_handler::DoGet() {
        std::string UUID = GetBinding("uuid","");
        ProvObjects::DeviceConfiguration   Existing;
        if(UUID.empty() || !DB_.GetRecord("id", UUID, Existing)) {
            NotFound();
            return;
        }

        std::string Arg;
        if(HasParameter("expandInUse",Arg) && Arg=="true") {
            Storage::ExpandedListMap    M;
            std::vector<std::string>    Errors;
            Poco::JSON::Object          Inner;
            if(Storage()->ExpandInUse(Existing.inUse,M,Errors)) {
                for(const auto &[type,list]:M) {
                    Poco::JSON::Array   ObjList;
                    for(const auto &i:list.entries) {
                        Poco::JSON::Object  O;
                        i.to_json(O);
                        ObjList.add(O);
                    }
                    Inner.set(type,ObjList);
                }
            }
            Poco::JSON::Object  Answer;
            Answer.set("entries", Inner);
            ReturnObject(Answer);
            return;
        }

        Poco::JSON::Object  Answer;
        Existing.to_json(Answer);
        ReturnObject(Answer);
    }

    void RESTAPI_configurations_handler::DoDelete() {
        std::string UUID = GetBinding("uuid","");
        ProvObjects::DeviceConfiguration   Existing;
        if(UUID.empty() || !DB_.GetRecord("id", UUID, Existing)) {
            NotFound();
            return;
        }

        if(!Existing.inUse.empty()) {
            BadRequest(RESTAPI::Errors::StillInUse);
            return;
        }

        if(DB_.DeleteRecord("id", UUID)) {
            OK();
            return;
        }
        InternalError(RESTAPI::Errors::CouldNotBeDeleted);
    }

    //      interfaces
    //      metrics
    //      radios
    //      services
    //      globals
    //      unit
    bool RESTAPI_configurations_handler::ValidateConfigBlock(const ProvObjects::DeviceConfiguration &Config, std::string & Error) {
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

            if(ValidateUCentralConfiguration(i.configuration, Error)) {
                /* nothing to do */ ;
            } else {
                // std::cout << "Block: " << std::endl << ">>>" << std::endl << i.configuration << std::endl << ">>> REJECTED" << std::endl;
                return false;
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

        std::string Arg;
        if(HasParameter("validateOnly",Arg) && Arg=="true") {
            auto Body = ParseStream();
            if(!Body->has("configuration")) {
                BadRequest("Must have 'configuration' element.");
                return;
            }
            auto Config=Body->get("configuration").toString();
            Poco::JSON::Object  Answer;
            std::string Error;
            auto Res = ValidateUCentralConfiguration(Config,Error);
            Answer.set("valid",Res);
            Answer.set("error", Error);
            ReturnObject(Answer);
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

        std::string Error;
        if(!ValidateConfigBlock(C,Error)) {
            return BadRequest(RESTAPI::Errors::ConfigBlockInvalid + ", error: " + Error);
        }

        if(DB_.CreateRecord(C)) {
            DB_.GetRecord("id", C.info.id, C);
            if(!C.managementPolicy.empty())
                Storage()->PolicyDB().AddInUse("id",C.managementPolicy,DB_.Prefix(), C.info.id);

            Poco::JSON::Object  Answer;
            C.to_json(Answer);
            ReturnObject(Answer);
            return;
        }
        InternalError(RESTAPI::Errors::RecordNotCreated);
    }

    void RESTAPI_configurations_handler::DoPut() {
        auto UUID = GetBinding("uuid","");
        ProvObjects::DeviceConfiguration    Existing;
        if(UUID.empty() || !DB_.GetRecord("id", UUID, Existing)) {
            return NotFound();
        }

        ProvObjects::DeviceConfiguration    NewConfig;
        auto ParsedObj = ParseStream();
        if (!NewConfig.from_json(ParsedObj)) {
            return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
        }

        if(!NewConfig.deviceTypes.empty() && !Storage()->AreAcceptableDeviceTypes(NewConfig.deviceTypes, true)) {
            return BadRequest(RESTAPI::Errors::InvalidDeviceTypes);
        }

        std::string Error;
        if(!ValidateConfigBlock( NewConfig,Error)) {
            return BadRequest(RESTAPI::Errors::ConfigBlockInvalid + ", error: " + Error);
        }

        if(ParsedObj->has("configuration")) {
            Existing.configuration = NewConfig.configuration;
        }

        for(auto &i:NewConfig.info.notes) {
            i.createdBy = UserInfo_.userinfo.email;
            Existing.info.notes.insert(Existing.info.notes.begin(),i);
        }

        std::string MovePolicy;
        bool        MovingPolicy=false;
        if(AssignIfPresent(ParsedObj,"managementPolicy",MovePolicy)) {
            if(!MovePolicy.empty() && !Storage()->PolicyDB().Exists("id",NewConfig.managementPolicy)) {
                BadRequest(RESTAPI::Errors::UnknownManagementPolicyUUID);
                return;
            }
            MovingPolicy = NewConfig.managementPolicy != Existing.managementPolicy;
        }

        if(!NewConfig.deviceTypes.empty())
            Existing.deviceTypes = NewConfig.deviceTypes;

        AssignIfPresent(ParsedObj, "name", Existing.info.name);
        AssignIfPresent(ParsedObj,"description", Existing.info.description);
        AssignIfPresent(ParsedObj, "rrm", Existing.rrm);
        Existing.info.modified = std::time(nullptr);
        AssignIfPresent(ParsedObj,"firmwareUpgrade",Existing.firmwareUpgrade);
        AssignIfPresent(ParsedObj,"firmwareRCOnly", Existing.firmwareRCOnly);

        if(!NewConfig.variables.empty())
            Existing.variables = NewConfig.variables;

        if(DB_.UpdateRecord("id",UUID,Existing)) {
            if(MovingPolicy) {
                if(!Existing.managementPolicy.empty())
                    Storage()->PolicyDB().DeleteInUse("id",Existing.managementPolicy,DB_.Prefix(),Existing.info.id);
                if(!MovePolicy.empty())
                    Storage()->PolicyDB().AddInUse("id",MovePolicy,DB_.Prefix(),Existing.info.id);
                Existing.managementPolicy = MovePolicy;
            }
            DB_.UpdateRecord("id", UUID, Existing);

            ProvObjects::DeviceConfiguration    D;
            DB_.GetRecord("id",UUID,D);
            Poco::JSON::Object  Answer;
            D.to_json(Answer);
            ReturnObject(Answer);
            return;
        }
        InternalError(RESTAPI::Errors::RecordNotUpdated);
    }
}