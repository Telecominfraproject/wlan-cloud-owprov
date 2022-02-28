//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//


#include "RESTAPI_entity_handler.h"

#include "RESTObjects/RESTAPI_ProvObjects.h"
#include "RESTObjects/RESTAPI_SecurityObjects.h"
#include "StorageService.h"
#include "RESTAPI_db_helpers.h"

namespace OpenWifi{

    void RESTAPI_entity_handler::DoGet() {
        std::string UUID = GetBinding("uuid", "");
        ProvObjects::Entity Existing;
        if(UUID.empty() || !DB_.GetRecord("id",UUID,Existing)) {
            return NotFound();
        }

        Poco::JSON::Object Answer;
        Existing.to_json(Answer);
        if(NeedAdditionalInfo())
            AddExtendedInfo( Existing, Answer);
        ReturnObject(Answer);
    }

    void RESTAPI_entity_handler::DoDelete() {
        std::string UUID = GetBinding("uuid", "");
        ProvObjects::Entity Existing;
        if(UUID.empty() || !DB_.GetRecord("id",UUID,Existing)) {
            return NotFound();
        }

        if(UUID == EntityDB::RootUUID()) {
            return BadRequest(RESTAPI::Errors::CannotDeleteRoot);
        }

        if( !Existing.children.empty() || !Existing.devices.empty() || !Existing.venues.empty() || !Existing.locations.empty()
            || !Existing.contacts.empty()) {
            return BadRequest(RESTAPI::Errors::StillInUse);
        }

        if(!Existing.deviceConfiguration.empty()) {
            for(auto &i:Existing.deviceConfiguration)
                StorageService()->ConfigurationDB().DeleteInUse("id", i, DB_.Prefix(), Existing.info.id);
        }

        if(DB_.DeleteRecord("id",UUID)) {
            DB_.DeleteChild("id",Existing.parent,UUID);
            return OK();
        }
        InternalError(RESTAPI::Errors::CouldNotBeDeleted);
    }

    void RESTAPI_entity_handler::DoPost() {
        std::string UUID = GetBinding("uuid", "");
        if(UUID.empty()) {
            return BadRequest(RESTAPI::Errors::MissingUUID);
        }

        if(!DB_.RootExists() && UUID != EntityDB::RootUUID()) {
            return BadRequest(RESTAPI::Errors::MustCreateRootFirst);
        }

        auto Obj = ParseStream();
        ProvObjects::Entity NewEntity;
        if (!NewEntity.from_json(Obj)) {
            return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
        }

        if(!ProvObjects::CreateObjectInfo(Obj,UserInfo_.userinfo,NewEntity.info)) {
            return BadRequest(RESTAPI::Errors::NameMustBeSet);
        }

        //  When creating an entity, it cannot have any relations other that parent, notes, name, description. Everything else
        //  must be conveyed through PUT.
        NewEntity.info.id = (UUID==EntityDB::RootUUID()) ? UUID : MicroService::CreateUUID();

        if(UUID==EntityDB::RootUUID()) {
            NewEntity.parent="";
        } else if(NewEntity.parent.empty() || !DB_.Exists("id",NewEntity.parent)) {
            return BadRequest(RESTAPI::Errors::ParentUUIDMustExist);
        }

        if(!NewEntity.managementPolicy.empty() && !StorageService()->PolicyDB().Exists("id", NewEntity.managementPolicy)){
            return BadRequest(RESTAPI::Errors::UnknownManagementPolicyUUID);
        }

        if(!NewEntity.sourceIP.empty() && !CIDR::ValidateIpRanges(NewEntity.sourceIP)) {
            return BadRequest(RESTAPI::Errors::InvalidIPRanges);
        }

        NewEntity.venues.clear();
        NewEntity.children.clear();
        NewEntity.contacts.clear();
        NewEntity.locations.clear();
        NewEntity.deviceConfiguration.clear();
        NewEntity.managementRoles.clear();

        if(DB_.CreateShortCut(NewEntity)) {
            MoveUsage(StorageService()->PolicyDB(),DB_,"",NewEntity.managementPolicy,NewEntity.info.id);
            if(UUID==EntityDB::RootUUID()) {
                DB_.CheckForRoot();
            } else {
                DB_.AddChild("id",NewEntity.parent,NewEntity.info.id);
            }

            Poco::JSON::Object  Answer;
            NewEntity.to_json(Answer);
            return ReturnObject(Answer);
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
            return NotFound();
        }

        auto RawObject = ParseStream();
        ProvObjects::Entity NewEntity;
        if(!NewEntity.from_json(RawObject)) {
            return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
        }

        if(!UpdateObjectInfo(RawObject, UserInfo_.userinfo, Existing.info)) {
            return BadRequest(RESTAPI::Errors::NameMustBeSet);
        }

        std::string NewManagementPolicy;
        Types::UUIDvec_t NewConfiguration;
        bool        MovingConfiguration=false,
                    MovingManagementPolicy=false;
        if(RawObject->has("deviceConfiguration")) {
            if(!NewEntity.deviceConfiguration.empty()) {
                for(auto &i:NewEntity.deviceConfiguration) {
                    if(!StorageService()->ConfigurationDB().Exists("id",i)) {
                        return BadRequest(RESTAPI::Errors::ConfigurationMustExist);
                    }
                }
                NewConfiguration = NewEntity.deviceConfiguration;
            }
            MovingConfiguration = Existing.deviceConfiguration != NewConfiguration;
        }
        if(AssignIfPresent(RawObject,"managementPolicy",NewManagementPolicy)) {
            if(!NewManagementPolicy.empty() && !StorageService()->PolicyDB().Exists("id",NewManagementPolicy)) {
                return BadRequest(RESTAPI::Errors::UnknownManagementPolicyUUID);
            }
            MovingManagementPolicy = Existing.managementPolicy != NewManagementPolicy;
        }

        if(RawObject->has("sourceIP")) {
            if(!NewEntity.sourceIP.empty() && !CIDR::ValidateIpRanges(NewEntity.sourceIP)) {
                return BadRequest(RESTAPI::Errors::InvalidIPRanges);
            }
            Existing.sourceIP = NewEntity.sourceIP;
        }

        std::string Error;
        if(!StorageService()->Validate(Parameters_,Error)) {
            return BadRequest(Error);
        }

        AssignIfPresent(RawObject, "rrm", Existing.rrm);

        if(DB_.UpdateRecord("id",UUID,Existing)) {

            if(MovingConfiguration) {
                if(!Existing.deviceConfiguration.empty())
                    for(auto &i:Existing.deviceConfiguration)
                        StorageService()->ConfigurationDB().DeleteInUse("id",i,DB_.Prefix(),Existing.info.id);
                if(!NewConfiguration.empty())
                    for(auto &i:NewConfiguration)
                        StorageService()->ConfigurationDB().AddInUse("id",i,DB_.Prefix(),Existing.info.id);
                Existing.deviceConfiguration = NewConfiguration;
            }

            if(MovingManagementPolicy) {
                if(!Existing.managementPolicy.empty())
                    StorageService()->PolicyDB().DeleteInUse("id",Existing.managementPolicy, DB_.Prefix(), Existing.info.id);
                if(!NewManagementPolicy.empty())
                    StorageService()->PolicyDB().AddInUse("id", NewManagementPolicy, DB_.Prefix(), Existing.info.id);
                Existing.managementPolicy = NewManagementPolicy;
            }

            DB_.UpdateRecord("id", Existing.info.id, Existing);

            Poco::JSON::Object  Answer;
            ProvObjects::Entity NewRecord;
            StorageService()->EntityDB().GetRecord("id",UUID, NewRecord);
            NewRecord.to_json(Answer);
            return ReturnObject(Answer);
        }
        InternalError(RESTAPI::Errors::RecordNotUpdated);
    }
}