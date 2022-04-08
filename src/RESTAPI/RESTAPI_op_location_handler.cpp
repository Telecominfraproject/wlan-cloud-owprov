//
// Created by stephane bourque on 2022-04-07.
//

#include "RESTAPI_op_location_handler.h"
#include "sdks/SDK_sec.h"
#include "RESTAPI/RESTAPI_db_helpers.h"

namespace OpenWifi {

    void RESTAPI_op_location_handler::DoGet() {
        auto uuid = GetBinding("uuid","");
        if(uuid.empty()) {
            return BadRequest(RESTAPI::Errors::MissingUUID);
        }
        OpLocationDB::RecordName  Existing;
        if(!DB_.GetRecord("id",uuid,Existing)) {
            return NotFound();
        }
        Poco::JSON::Object  Answer;
        Existing.to_json(Answer);
        return ReturnObject(Answer);
    }

    void RESTAPI_op_location_handler::DoDelete() {
        auto uuid = GetBinding("uuid","");
        if(uuid.empty()) {
            return BadRequest(RESTAPI::Errors::MissingUUID);
        }

        OpLocationDB::RecordName  Existing;
        if(!DB_.GetRecord("id",uuid,Existing)) {
            return NotFound();
        }

        // see if anyone is still using this thing
        if(!Existing.subscriberId.empty()){
            return BadRequest(RESTAPI::Errors::StillInUse);
        }
        DB_.DeleteRecord("id", uuid);
        return OK();
    }

    void RESTAPI_op_location_handler::DoPost() {

        auto RawObject = ParseStream();
        OpLocationDB::RecordName   NewObject;
        if(!NewObject.from_json(RawObject)) {
            return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
        }

        if(RawObject->has("type")) {
            if(!NewObject.type.empty()) {
                if (ValidLocationType(NewObject.type)) {
                    return BadRequest(RESTAPI::Errors::InvalidLocationType);
                }
                NewObject.type = Poco::toLower(NewObject.type);
            } else {
                NewObject.type.clear();
            }
        }

        if(NewObject.operatorId.empty() || !StorageService()->OperatorDB().Exists("id",NewObject.operatorId)) {
            return BadRequest(RESTAPI::Errors::MissingUUID);
        }

        if(!NewObject.subscriberId.empty()) {
            SecurityObjects::UserInfo   NewSubInfo;
            if(!SDK::Sec::Subscriber::Get(this, NewObject.subscriberId, NewSubInfo)) {
                return BadRequest(RESTAPI::Errors::InvalidSubscriberId);
            }
        }

        if(RawObject->has("managementPolicy") && !StorageService()->PolicyDB().Exists("id",NewObject.managementPolicy)) {
            return BadRequest(RESTAPI::Errors::UnknownManagementPolicyUUID);
        }

        ProvObjects::CreateObjectInfo(RawObject, UserInfo_.userinfo, NewObject.info);
        if(DB_.CreateRecord(NewObject)) {
            OpLocationDB::RecordName   New;
            StorageService()->OpLocationDB().GetRecord("id", NewObject.info.id, New);
            Poco::JSON::Object  Answer;
            New.to_json(Answer);
            return ReturnObject(Answer);
        }
        return InternalError("Location could not be created.");
    }

    void RESTAPI_op_location_handler::DoPut() {
        auto uuid = GetBinding("uuid","");

        OpLocationDB::RecordName   Existing;
        if(uuid.empty() || DB_.GetRecord("id",uuid,Existing)) {
            return BadRequest(RESTAPI::Errors::MissingUUID);
        }

        auto RawObject = ParseStream();
        OpLocationDB::RecordName   UpdateObj;
        if(!UpdateObj.from_json(RawObject)) {
            return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
        }

        if(RawObject->has("type")) {
            if(!UpdateObj.type.empty()) {
                if (ValidLocationType(UpdateObj.type)) {
                    return BadRequest(RESTAPI::Errors::InvalidLocationType);
                }
                Existing.type = Poco::toLower(UpdateObj.type);
            } else {
                Existing.type.clear();
            }
        }

        if(RawObject->has("managementPolicy") && !StorageService()->PolicyDB().Exists("id",UpdateObj.managementPolicy)) {
            return BadRequest(RESTAPI::Errors::UnknownManagementPolicyUUID);
        }

        if(!UpdateObj.subscriberId.empty()) {
            SecurityObjects::UserInfo   NewSubInfo;
            if(!SDK::Sec::Subscriber::Get(this, UpdateObj.subscriberId, NewSubInfo)) {
                return BadRequest(RESTAPI::Errors::InvalidSubscriberId);
            }
        } else {
            if(RawObject->has("subscriberId") && UpdateObj.subscriberId.empty())
                Existing.subscriberId.clear();
        }

        ProvObjects::UpdateObjectInfo(RawObject, UserInfo_.userinfo, Existing.info);

        AssignIfPresent(RawObject,"buildingName",Existing.buildingName);
        AssignIfPresent(RawObject,"addressLines",Existing.addressLines);
        AssignIfPresent(RawObject,"city",Existing.city);
        AssignIfPresent(RawObject,"state",Existing.state);
        AssignIfPresent(RawObject,"postal",Existing.postal);
        AssignIfPresent(RawObject,"country",Existing.country);
        AssignIfPresent(RawObject,"mobiles",Existing.mobiles);
        AssignIfPresent(RawObject,"phones",Existing.phones);
        AssignIfPresent(RawObject,"geoCode",Existing.geoCode);

        if(DB_.UpdateRecord("id",uuid,Existing)) {
            OpLocationDB::RecordName   New;
            StorageService()->OpLocationDB().GetRecord("id", Existing.info.id, New);
            Poco::JSON::Object  Answer;
            New.to_json(Answer);
            return ReturnObject(Answer);
        }
        return InternalError("Location could not be updated.");
    }
}