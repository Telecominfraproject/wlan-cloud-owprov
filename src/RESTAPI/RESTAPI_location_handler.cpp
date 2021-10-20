//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//


#include "RESTAPI_location_handler.h"
#include "framework/RESTAPI_errors.h"

#include "RESTAPI_ProvObjects.h"
#include "StorageService.h"
#include "framework/RESTAPI_protocol.h"
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
            Answer.set("entries", Inner);
            return ReturnObject(Answer);
        } else if(QB_.AdditionalInfo) {
            AddLocationExtendedInfo(Existing, Answer);
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

        if(DB_.DeleteRecord("id",UUID)) {
            return OK();
        }
        InternalError(RESTAPI::Errors::CouldNotBeDeleted);
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

        if(NewObject.entity.empty() || !Storage()->EntityDB().Exists("id",NewObject.entity)) {
            return BadRequest(RESTAPI::Errors::EntityMustExist);
        }

        if(!NewObject.managementPolicy.empty() && !Storage()->PolicyDB().Exists("id",NewObject.managementPolicy)) {
            return BadRequest(RESTAPI::Errors::UnknownManagementPolicyUUID);
        }

        NewObject.info.id = Daemon()->CreateUUID();
        NewObject.info.created = NewObject.info.modified = std::time(nullptr);
        NewObject.inUse.clear();

        if(DB_.CreateRecord(NewObject)) {
            if(!NewObject.managementPolicy.empty())
                Storage()->PolicyDB().AddInUse("id",NewObject.managementPolicy,DB_.Prefix(),NewObject.info.id);

            Storage()->EntityDB().AddLocation("id",NewObject.entity,NewObject.info.id);

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

        std::string MoveFromPolicy,MoveToPolicy;
        bool MovingPolicy=false;
        if(AssignIfPresent(RawObject,"managementPolicy",MoveToPolicy)) {
            if(!MoveToPolicy.empty() && !Storage()->PolicyDB().Exists("id",MoveToPolicy)) {
                return BadRequest(RESTAPI::Errors::UnknownManagementPolicyUUID);
            }
            MoveFromPolicy = Existing.managementPolicy;
            MovingPolicy = MoveToPolicy != MoveFromPolicy;
            Existing.managementPolicy = MoveToPolicy;
        }

        std::string MoveFromEntity,MoveToEntity;
        bool MovingEntity=false;
        if(AssignIfPresent(RawObject,"entity",MoveToEntity)) {
            if(!MoveToEntity.empty() || !Storage()->EntityDB().Exists("id",MoveToEntity)) {
                return BadRequest(RESTAPI::Errors::EntityMustExist);
            }
            MoveFromEntity = Existing.entity;
            MovingEntity = MoveToEntity != Existing.entity;
            Existing.entity = MoveToEntity;
        }

        for(auto &i:NewObject.info.notes) {
            i.createdBy = UserInfo_.userinfo.email;
            Existing.info.notes.insert(Existing.info.notes.begin(),i);
        }

        AssignIfPresent(RawObject,"name",Existing.info.name);
        AssignIfPresent(RawObject,"description",Existing.info.description);
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
            if(MovingPolicy) {
                if(!MoveFromPolicy.empty())
                    Storage()->PolicyDB().DeleteInUse("id",MoveFromPolicy,DB_.Prefix(),Existing.info.id);
                if(!MoveToPolicy.empty())
                    Storage()->PolicyDB().AddInUse("id", MoveToPolicy, DB_.Prefix(), Existing.info.id);
            }
            if(MovingEntity) {
                if(!MoveFromEntity.empty())
                    Storage()->EntityDB().DeleteLocation("id",MoveFromEntity,Existing.info.id);
                if(!MoveToEntity.empty())
                    Storage()->EntityDB().AddLocation("id", MoveToEntity, Existing.info.id);
            }

            ProvObjects::Location    NewObjectAdded;
            DB_.GetRecord("id", UUID, NewObjectAdded);

            Poco::JSON::Object  Answer;
            NewObjectAdded.to_json(Answer);
            return ReturnObject(Answer);
        }
        InternalError(RESTAPI::Errors::RecordNotUpdated);
    }
}