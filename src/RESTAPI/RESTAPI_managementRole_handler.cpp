//
// Created by stephane bourque on 2021-08-26.
//

#include "RESTAPI_managementRole_handler.h"

#include "RESTObjects/RESTAPI_ProvObjects.h"
#include "StorageService.h"
#include "Poco/JSON/Parser.h"
#include "Poco/StringTokenizer.h"
#include "RESTAPI/RESTAPI_db_helpers.h"

namespace OpenWifi{

    void RESTAPI_managementRole_handler::DoGet() {
        ProvObjects::ManagementRole   Existing;
        std::string UUID = GetBinding(RESTAPI::Protocol::ID,"");
        if(UUID.empty() || !DB_.GetRecord(RESTAPI::Protocol::ID,UUID,Existing)) {
            return NotFound();
        }

        Poco::JSON::Object  Answer;
        std::string Arg;
        if(HasParameter("expandInUse",Arg) && Arg=="true") {
            Storage::ExpandedListMap    M;
            std::vector<std::string>    Errors;
            Poco::JSON::Object  Inner;
            if(StorageService()->ExpandInUse(Existing.inUse,M,Errors)) {
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
            Answer.set("entries", Inner);
            return ReturnObject(Answer);
        }

        if(QB_.AdditionalInfo)
            AddExtendedInfo(Existing,Answer);
        Existing.to_json(Answer);
        ReturnObject(Answer);
    }

    void RESTAPI_managementRole_handler::DoDelete() {
        ProvObjects::ManagementRole   Existing;
        std::string UUID = GetBinding(RESTAPI::Protocol::ID,"");
        if(UUID.empty() || !DB_.GetRecord(RESTAPI::Protocol::ID,UUID,Existing)) {
            return NotFound();
        }

        bool Force=false;
        std::string Arg;
        if(HasParameter("force",Arg) && Arg=="true")
            Force=true;

        if(!Force && !Existing.inUse.empty()) {
            return BadRequest(RESTAPI::Errors::StillInUse);
        }

        DB_.DeleteRecord("id", Existing.info.id);
        MoveUsage(StorageService()->PolicyDB(),DB_,Existing.managementPolicy,"",Existing.info.id);
        RemoveMembership(StorageService()->EntityDB(),&ProvObjects::Entity::managementRoles,Existing.entity,Existing.info.id);
        return OK();
    }

    void RESTAPI_managementRole_handler::DoPost() {
        std::string UUID = GetBinding(RESTAPI::Protocol::ID,"");
        if(UUID.empty()) {
            return BadRequest(RESTAPI::Errors::MissingUUID);
        }

        auto RawObj = ParseStream();
        ProvObjects::ManagementRole NewObject;
        if (!NewObject.from_json(RawObj)) {
            return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
        }

        if(!CreateObjectInfo(RawObj, UserInfo_.userinfo, NewObject.info)) {
            return BadRequest( RESTAPI::Errors::NameMustBeSet);
        }

        if(NewObject.entity.empty() || !StorageService()->EntityDB().Exists("id",NewObject.entity)) {
            return BadRequest(RESTAPI::Errors::EntityMustExist);
        }

        if(!NewObject.managementPolicy.empty() && !StorageService()->PolicyDB().Exists("id",NewObject.managementPolicy)) {
            return BadRequest(RESTAPI::Errors::UnknownManagementPolicyUUID);
        }

        if(DB_.CreateRecord(NewObject)) {
            AddMembership(StorageService()->EntityDB(),&ProvObjects::Entity::managementRoles,NewObject.entity,NewObject.info.id);
            MoveUsage(StorageService()->PolicyDB(), DB_, "", NewObject.managementPolicy, NewObject.info.id);

            Poco::JSON::Object Answer;
            ProvObjects::ManagementRole Role;
            DB_.GetRecord("id", NewObject.info.id,Role);
            Role.to_json(Answer);
            return ReturnObject(Answer);
        }
        InternalError(RESTAPI::Errors::RecordNotCreated);
    }

    void RESTAPI_managementRole_handler::DoPut() {
        ProvObjects::ManagementRole   Existing;
        std::string UUID = GetBinding(RESTAPI::Protocol::ID,"");
        if(UUID.empty() || !DB_.GetRecord(RESTAPI::Protocol::ID,UUID,Existing)) {
            return NotFound();
        }

        auto RawObject = ParseStream();
        ProvObjects::ManagementRole NewObject;
        if(!NewObject.from_json(RawObject)) {
            return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
        }

        if(!UpdateObjectInfo(RawObject, UserInfo_.userinfo, Existing.info)) {
            return BadRequest( RESTAPI::Errors::NameMustBeSet);
        }

        std::string MoveToPolicy,MoveFromPolicy;
        if(AssignIfPresent(RawObject,"managementPolicy",MoveToPolicy)) {
            if(!MoveToPolicy.empty() && !StorageService()->PolicyDB().Exists("id",MoveToPolicy)) {
                return BadRequest(RESTAPI::Errors::UnknownManagementPolicyUUID);
            }
            MoveFromPolicy = Existing.managementPolicy;
            Existing.managementPolicy = MoveToPolicy;
        }

        std::string MoveToEntity,MoveFromEntity;
        if(AssignIfPresent(RawObject,"entity",MoveToEntity)) {
            if(!MoveToEntity.empty() && !StorageService()->EntityDB().Exists("id",MoveToEntity)) {
                return BadRequest(RESTAPI::Errors::EntityMustExist);
            }
            MoveFromEntity = Existing.entity;
            Existing.entity = MoveToEntity;
        }

        if(DB_.UpdateRecord("id",UUID,Existing)) {
            MoveUsage(StorageService()->PolicyDB(),DB_,MoveFromPolicy, MoveToPolicy, Existing.info.id);
            ManageMembership(StorageService()->EntityDB(),&ProvObjects::Entity::managementRoles, MoveFromEntity, MoveToEntity, Existing.info.id);

            ProvObjects::ManagementRole NewRecord;

            DB_.GetRecord("id", UUID, NewRecord);
            Poco::JSON::Object  Answer;
            NewRecord.to_json(Answer);
            return ReturnObject(Answer);
        }
        InternalError(RESTAPI::Errors::RecordNotUpdated);
    }
}