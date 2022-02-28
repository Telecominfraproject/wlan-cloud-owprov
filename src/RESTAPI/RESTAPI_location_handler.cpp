//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//


#include "RESTAPI_location_handler.h"
#include "RESTObjects/RESTAPI_ProvObjects.h"
#include "StorageService.h"
#include "Daemon.h"
#include "RESTAPI/RESTAPI_db_helpers.h"

namespace OpenWifi{

    void RESTAPI_location_handler::DoGet() {
        std::string UUID = GetBinding("uuid","");
        ProvObjects::Location   Existing;
        if(UUID.empty() || !DB_.GetRecord("id", UUID, Existing)) {
            return BadRequest(RESTAPI::Errors::MissingUUID);
        }

        std::string Arg;
        Poco::JSON::Object  Answer;
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
        } else if(QB_.AdditionalInfo) {
            AddExtendedInfo(Existing, Answer);
        }
        Existing.to_json(Answer);
        ReturnObject(Answer);
    }

    void RESTAPI_location_handler::DoDelete() {
        std::string UUID = GetBinding("uuid","");
        ProvObjects::Location   Existing;
        if(UUID.empty() || !DB_.GetRecord("id", UUID, Existing)) {
            return NotFound();
        }

        bool Force=false;
        std::string Arg;
        if(HasParameter("force",Arg) && Arg=="true")
            Force=true;

        if(!Force && !Existing.inUse.empty()) {
            return BadRequest(RESTAPI::Errors::StillInUse);
        }

        DB_.DeleteRecord("id",UUID);
        ManageMembership(StorageService()->EntityDB(),&ProvObjects::Entity::locations,Existing.entity,"",Existing.info.id);
        return OK();
    }

    void RESTAPI_location_handler::DoPost() {
        std::string UUID = GetBinding(RESTAPI::Protocol::UUID,"");
        if(UUID.empty()) {
            return BadRequest(RESTAPI::Errors::MissingUUID);
        }

        auto Obj = ParseStream();
        ProvObjects::Location NewObject;
        if (!NewObject.from_json(Obj)) {
            return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
        }

        if(!ProvObjects::CreateObjectInfo(Obj, UserInfo_.userinfo, NewObject.info)) {
            return BadRequest( RESTAPI::Errors::NameMustBeSet);
        }

        if(NewObject.entity.empty() || !StorageService()->EntityDB().Exists("id",NewObject.entity)) {
            return BadRequest(RESTAPI::Errors::EntityMustExist);
        }

        if(!NewObject.managementPolicy.empty() && !StorageService()->PolicyDB().Exists("id",NewObject.managementPolicy)) {
            return BadRequest(RESTAPI::Errors::UnknownManagementPolicyUUID);
        }

        NewObject.inUse.clear();

        if(DB_.CreateRecord(NewObject)) {
            if(!NewObject.managementPolicy.empty())
                StorageService()->PolicyDB().AddInUse("id",NewObject.managementPolicy,DB_.Prefix(),NewObject.info.id);

            StorageService()->EntityDB().AddLocation("id",NewObject.entity,NewObject.info.id);

            Poco::JSON::Object Answer;
            NewObject.to_json(Answer);
            return ReturnObject(Answer);
        }
        BadRequest(RESTAPI::Errors::RecordNotCreated);
    }

    void RESTAPI_location_handler::DoPut() {
        std::string UUID = GetBinding(RESTAPI::Protocol::UUID,"");
        ProvObjects::Location   Existing;
        if(UUID.empty() || !DB_.GetRecord("id", UUID, Existing)) {
            return NotFound();
        }

        auto RawObject = ParseStream();
        ProvObjects::Location NewObject;
        if (!NewObject.from_json(RawObject)) {
            return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
        }

        if(!UpdateObjectInfo(RawObject, UserInfo_.userinfo, Existing.info)) {
            return BadRequest( RESTAPI::Errors::NameMustBeSet);
        }

        std::string MoveFromPolicy,MoveToPolicy;
        if(AssignIfPresent(RawObject,"managementPolicy",MoveToPolicy)) {
            if(!MoveToPolicy.empty() && !StorageService()->PolicyDB().Exists("id",MoveToPolicy)) {
                return BadRequest(RESTAPI::Errors::UnknownManagementPolicyUUID);
            }
            MoveFromPolicy = Existing.managementPolicy;
            Existing.managementPolicy = MoveToPolicy;
        }

        std::string MoveFromEntity,MoveToEntity;
        if(AssignIfPresent(RawObject,"entity",MoveToEntity)) {
            if(!MoveToEntity.empty() && !StorageService()->EntityDB().Exists("id",MoveToEntity)) {
                return BadRequest(RESTAPI::Errors::EntityMustExist);
            }
            MoveFromEntity = Existing.entity;
            Existing.entity = MoveToEntity;
        }

        AssignIfPresent(RawObject, "buildingName", Existing.buildingName);
        AssignIfPresent(RawObject, "city", Existing.city);
        AssignIfPresent(RawObject, "state", Existing.state);
        AssignIfPresent(RawObject, "postal", Existing.postal);
        AssignIfPresent(RawObject, "country", Existing.country);
        AssignIfPresent(RawObject, "geoCode", Existing.geoCode);
        if(RawObject->has("addressLines"))
            Existing.addressLines = NewObject.addressLines;
        if(RawObject->has("phones"))
            Existing.phones = NewObject.phones;
        if(RawObject->has("mobiles"))
            Existing.mobiles = NewObject.mobiles;
        Existing.info.modified = std::time(nullptr);
        if(RawObject->has("type"))
            Existing.type = NewObject.type;

        if(DB_.UpdateRecord("id", UUID, Existing)) {

            MoveUsage(StorageService()->PolicyDB(), DB_, MoveFromPolicy, MoveToPolicy, Existing.info.id);
            ManageMembership(Storage().EntityDB(),&ProvObjects::Entity::locations,MoveFromEntity, MoveToEntity, Existing.info.id);

            ProvObjects::Location    NewObjectAdded;
            DB_.GetRecord("id", UUID, NewObjectAdded);
            Poco::JSON::Object  Answer;
            NewObjectAdded.to_json(Answer);
            return ReturnObject(Answer);
        }
        InternalError(RESTAPI::Errors::RecordNotUpdated);
    }
}