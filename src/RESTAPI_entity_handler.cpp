//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//


#include "RESTAPI_entity_handler.h"
#include "RESTAPI_ProvObjects.h"
#include "StorageService.h"

#include "Poco/JSON/Parser.h"
#include "Daemon.h"
#include "RESTAPI_SecurityObjects.h"
#include "RESTAPI_utils.h"
#include "RESTAPI_errors.h"
#include "CIDRUtils.h"

namespace OpenWifi{

    void RESTAPI_entity_handler::DoGet() {
        std::string UUID = GetBinding("uuid", ""), Arg;
        bool AdditionalInfo = HasParameter("withExtendedInfo",Arg) && Arg=="true";

        ProvObjects::Entity Existing;
        if(UUID.empty() || !DB_.GetRecord("id",UUID,Existing)) {
            NotFound();
            return;
        }

        Poco::JSON::Object Answer;
        if(AdditionalInfo) {
            Poco::JSON::Object  EI;
            if(!Existing.managementPolicy.empty()) {
                Poco::JSON::Object  PolObj;
                ProvObjects::ManagementPolicy Policy;
                if(Storage()->PolicyDB().GetRecord("id",Existing.managementPolicy,Policy)) {
                    PolObj.set( "name", Policy.info.name);
                    PolObj.set( "description", Policy.info.description);
                }
                EI.set("managementPolicy",PolObj);
            }
            if(!Existing.deviceConfiguration.empty()) {
                Poco::JSON::Object  EntObj;
                ProvObjects::DeviceConfiguration DevConf;
                if(Storage()->ConfigurationDB().GetRecord("id",Existing.deviceConfiguration,DevConf)) {
                    EntObj.set( "name", DevConf.info.name);
                    EntObj.set( "description", DevConf.info.description);
                }
                EI.set("deviceConfiguration",EntObj);
            }
            Answer.set("extendedInfo", EI);
        }
        Existing.to_json(Answer);
        ReturnObject(Answer);
    }

    void RESTAPI_entity_handler::DoDelete() {
        std::string UUID = GetBinding("uuid", "");
        ProvObjects::Entity Existing;
        if(UUID.empty() || !DB_.GetRecord("id",UUID,Existing)) {
            NotFound();
            return;
        }

        if(UUID == EntityDB::RootUUID()) {
            BadRequest(RESTAPI::Errors::CannotDeleteRoot);
            return;
        }

        if(!Existing.children.empty() || !Existing.devices.empty() || !Existing.venues.empty()) {
            BadRequest(RESTAPI::Errors::StillInUse);
            return;
        }

        if(!Existing.deviceConfiguration.empty())
            Storage()->ConfigurationDB().DeleteInUse("id", Existing.deviceConfiguration, DB_.Prefix(), Existing.info.id);

        for(auto &i:Existing.locations) {
            Storage()->LocationDB().DeleteInUse("id", i, DB_.Prefix(), Existing.info.id);
        }

        for(auto &i:Existing.contacts) {
            Storage()->ContactDB().DeleteInUse("id", i, DB_.Prefix(), Existing.info.id);
        }

        if(DB_.DeleteRecord("id",UUID)) {
            DB_.DeleteChild("id",Existing.parent,UUID);
            OK();
            return;
        }
        InternalError(RESTAPI::Errors::CouldNotBeDeleted);
    }

    void RESTAPI_entity_handler::DoPost() {
        std::string UUID = GetBinding("uuid", "");
        if(UUID.empty()) {
            BadRequest(RESTAPI::Errors::MissingUUID);
            return;
        }

        if(!DB_.RootExists() && UUID != EntityDB::RootUUID()) {
            BadRequest(RESTAPI::Errors::MustCreateRootFirst);
            return;
        }

        auto Obj = ParseStream();
        ProvObjects::Entity NewEntity;
        if (!NewEntity.from_json(Obj)) {
            BadRequest(RESTAPI::Errors::InvalidJSONDocument);
            return;
        }
        NewEntity.info.modified = NewEntity.info.created = std::time(nullptr);

        for(auto &i:NewEntity.info.notes)
            i.createdBy = UserInfo_.userinfo.email;

        //  When creating an entity, it cannot have any relations other that parent, notes, name, description. Everything else
        //  must be conveyed through PUT.
        NewEntity.info.id = (UUID==EntityDB::RootUUID()) ? UUID : Daemon()->CreateUUID() ;
        if(UUID==EntityDB::RootUUID()) {
            NewEntity.parent="";
        } else if(NewEntity.parent.empty() || !DB_.Exists("id",NewEntity.parent)) {
            BadRequest(RESTAPI::Errors::ParentUUIDMustExist);
            return;
        }

        if(!NewEntity.deviceConfiguration.empty() && !Storage()->ConfigurationDB().Exists("id",NewEntity.deviceConfiguration)) {
            BadRequest(RESTAPI::Errors::ConfigurationMustExist);
            return;
        }

        if(!NewEntity.managementPolicy.empty() && !Storage()->PolicyDB().Exists("id", NewEntity.managementPolicy)){
            BadRequest(RESTAPI::Errors::UnknownManagementPolicyUUID);
            return;
        }

        if(!NewEntity.sourceIP.empty() && !CIDR::ValidateIpRanges(NewEntity.sourceIP)) {
            BadRequest(RESTAPI::Errors::InvalidIPRanges);
            return;
        }

        NewEntity.venues.clear();
        NewEntity.children.clear();
        NewEntity.contacts.clear();
        NewEntity.locations.clear();

        if(DB_.CreateShortCut(NewEntity)) {
            if(UUID==EntityDB::RootUUID()) {
                DB_.CheckForRoot();
            } else {
                DB_.AddChild("id",NewEntity.parent,NewEntity.info.id);
            }

            Poco::JSON::Object  Answer;
            NewEntity.to_json(Answer);
            ReturnObject(Answer);
            return;
        }
        InternalError(RESTAPI::Errors::RecordNotCreated);
    }

    /*
     * Put is a complex operation, it contains commands only.
     *      addContact=UUID, delContact=UUID,
     *      addLocation=UUID, delLocation=UUID,
     *      addVenue=UUID, delVenue=UUID,
     *      addEntity=UUID, delEntity=UUID
     *      addDevice=UUID, delDevice=UUID
     */

    void RESTAPI_entity_handler::DoPut() {
        std::string UUID = GetBinding("uuid", "");
        ProvObjects::Entity Existing;
        if(UUID.empty() || !DB_.GetRecord("id",UUID,Existing)) {
            NotFound();
            return;
        }

        auto RawObject = ParseStream();
        ProvObjects::Entity NewEntity;
        if(!NewEntity.from_json(RawObject)) {
            BadRequest(RESTAPI::Errors::InvalidJSONDocument);
            return;
        }

        std::string NewConfiguration, NewManagementPolicy;
        bool        MovingConfiguration=false,
                    MovingManagementPolicy=false;
        if(AssignIfPresent(RawObject,"deviceConfiguration",NewConfiguration)) {
            if(!NewConfiguration.empty() && !Storage()->ConfigurationDB().Exists("id",NewConfiguration)) {
                BadRequest(RESTAPI::Errors::ConfigurationMustExist);
                return;
            }
            MovingConfiguration = Existing.deviceConfiguration != NewConfiguration;
        }
        if(AssignIfPresent(RawObject,"managementPolicy",NewManagementPolicy)) {
            if(!NewManagementPolicy.empty() && !Storage()->PolicyDB().Exists("id",NewManagementPolicy)) {
                BadRequest(RESTAPI::Errors::UnknownManagementPolicyUUID);
                return;
            }
            MovingManagementPolicy = Existing.managementPolicy != NewManagementPolicy;
        }

        if(RawObject->has("sourceIP")) {
            if(!NewEntity.sourceIP.empty() && !CIDR::ValidateIpRanges(NewEntity.sourceIP)) {
                BadRequest(RESTAPI::Errors::InvalidIPRanges);
                return;
            }
            Existing.sourceIP = NewEntity.sourceIP;
        }

        for(auto &i:NewEntity.info.notes) {
            i.createdBy = UserInfo_.userinfo.email;
            Existing.info.notes.insert(Existing.info.notes.begin(),i);
        }

        std::string Error;
        if(!Storage()->Validate(Parameters_,Error)) {
            BadRequest(Error);
            return;
        }

        AssignIfPresent(RawObject, "rrm", Existing.rrm);
        AssignIfPresent(RawObject, "name", Existing.info.name);
        AssignIfPresent(RawObject, "description", Existing.info.description);

        Existing.info.modified = std::time(nullptr);
        if(DB_.UpdateRecord("id",UUID,Existing)) {
            for(const auto &i:*Request) {
                std::string Child{i.second};
                auto UUID_parts = Utils::Split(Child,':');
                if(i.first=="add" && UUID_parts[0] == "con") {
                    DB_.AddContact("id", UUID, UUID_parts[1]);
                    Storage()->ContactDB().AddInUse("id",UUID_parts[1],DB_.Prefix(), UUID);
                } else if (i.first == "del" && UUID_parts[0] == "con") {
                    DB_.DeleteContact("id", UUID, UUID_parts[1]);
                    Storage()->ContactDB().DeleteInUse("id",UUID_parts[1],DB_.Prefix(),UUID);
                } else if (i.first == "add" && UUID_parts[0] == "loc") {
                    DB_.AddLocation("id", UUID, UUID_parts[1]);
                    Storage()->LocationDB().AddInUse("id",UUID_parts[1],DB_.Prefix(),UUID);
                } else if (i.first == "del" && UUID_parts[0] == "loc") {
                    DB_.DeleteLocation("id", UUID, UUID_parts[1]);
                    Storage()->LocationDB().DeleteInUse("id",UUID_parts[1],DB_.Prefix(),UUID);
                }
            }

            if(MovingConfiguration) {
                if(!Existing.deviceConfiguration.empty())
                    Storage()->ConfigurationDB().DeleteInUse("id",Existing.deviceConfiguration,DB_.Prefix(),Existing.info.id);
                if(!NewConfiguration.empty())
                    Storage()->ConfigurationDB().AddInUse("id",NewConfiguration,DB_.Prefix(),Existing.info.id);
                Existing.deviceConfiguration = NewConfiguration;
            }

            if(MovingManagementPolicy) {
                if(!Existing.managementPolicy.empty())
                    Storage()->PolicyDB().DeleteInUse("id",Existing.managementPolicy, DB_.Prefix(), Existing.info.id);
                if(!NewManagementPolicy.empty())
                    Storage()->PolicyDB().AddInUse("id", NewManagementPolicy, DB_.Prefix(), Existing.info.id);
                Existing.managementPolicy = NewManagementPolicy;
            }

            DB_.UpdateRecord("id", Existing.info.id, Existing);

            Poco::JSON::Object  Answer;
            ProvObjects::Entity NewRecord;
            Storage()->EntityDB().GetRecord("id",UUID, NewRecord);
            NewRecord.to_json(Answer);
            ReturnObject(Answer);
            return;
        }
        InternalError(RESTAPI::Errors::RecordNotUpdated);
    }
}