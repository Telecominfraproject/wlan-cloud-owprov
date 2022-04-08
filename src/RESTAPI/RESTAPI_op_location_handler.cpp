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

        if( !ValidDbId(NewObject.operatorId,StorageService()->OperatorDB(), false, RESTAPI::Errors::InvalidOperatorId, *this ) ||
            !ValidDbId(NewObject.managementPolicy,StorageService()->PolicyDB(), true, RESTAPI::Errors::UnknownManagementPolicyUUID, *this ) ||
            !ValidSubscriberId(NewObject.subscriberId,true, *this)  ||
            !ValidLocationType(NewObject.type,*this)) {
            return;
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

        if( !ValidLocationType(UpdateObj.type,*this)     ||
            !ValidDbId(UpdateObj.managementPolicy,StorageService()->PolicyDB(), true, RESTAPI::Errors::UnknownManagementPolicyUUID, *this ) ||
            !ValidSubscriberId(UpdateObj.subscriberId,true,*this)
            ) {
            return;
        }

        ProvObjects::UpdateObjectInfo(RawObject, UserInfo_.userinfo, Existing.info);
        AssignIfPresent(RawObject,"type",Existing.type);
        AssignIfPresent(RawObject,"subscriberId", Existing.subscriberId);
        AssignIfPresent(RawObject,"managementPolicy", Existing.managementPolicy);
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