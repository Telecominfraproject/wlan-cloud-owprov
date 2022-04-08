//
// Created by stephane bourque on 2022-04-07.
//

#include "RESTAPI_op_contact_handler.h"
#include "RESTAPI_db_helpers.h"
#include "sdks/SDK_sec.h"

namespace OpenWifi {

    void RESTAPI_op_contact_handler::DoGet() {
        auto uuid = GetBinding("uuid","");
        if(uuid.empty()) {
            return BadRequest(RESTAPI::Errors::MissingUUID);
        }
        OpContactDB::RecordName  Existing;
        if(!DB_.GetRecord("id",uuid,Existing)) {
            return NotFound();
        }
        Poco::JSON::Object  Answer;
        Existing.to_json(Answer);
        return ReturnObject(Answer);
    }

    void RESTAPI_op_contact_handler::DoDelete() {
        auto uuid = GetBinding("uuid","");
        if(uuid.empty()) {
            return BadRequest(RESTAPI::Errors::MissingUUID);
        }

        OpContactDB::RecordName  Existing;
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

    void RESTAPI_op_contact_handler::DoPost() {

        auto RawObject = ParseStream();
        OpContactDB::RecordName   NewObject;
        if(!NewObject.from_json(RawObject)) {
            return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
        }

        if( !ValidDbId(NewObject.operatorId,StorageService()->OperatorDB(), false, RESTAPI::Errors::InvalidOperatorId, *this ) ||
            !ValidDbId(NewObject.managementPolicy,StorageService()->PolicyDB(), false, RESTAPI::Errors::UnknownManagementPolicyUUID, *this ) ||
            !ValidContactType(NewObject.type,*this) ||
            !ValidSubscriberId(NewObject.subscriberId,true,NewObject.primaryEmail,*this)) {
            return;
        }

        ProvObjects::CreateObjectInfo(RawObject, UserInfo_.userinfo, NewObject.info);
        if(DB_.CreateRecord(NewObject)) {
            OpContactDB::RecordName   New;
            StorageService()->OpContactDB().GetRecord("id", NewObject.info.id, New);
            Poco::JSON::Object  Answer;
            New.to_json(Answer);
            return ReturnObject(Answer);
        }
        return InternalError("Contact could not be created.");
    }

    void RESTAPI_op_contact_handler::DoPut() {
        auto uuid = GetBinding("uuid","");

        OpContactDB::RecordName   Existing;
        if(uuid.empty() || DB_.GetRecord("id",uuid,Existing)) {
            return BadRequest(RESTAPI::Errors::MissingUUID);
        }

        auto RawObject = ParseStream();
        OpContactDB::RecordName   UpdateObj;
        if(!UpdateObj.from_json(RawObject)) {
            return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
        }

        if( !ValidContactType(UpdateObj.type,*this)     ||
            !ValidDbId(UpdateObj.managementPolicy,StorageService()->PolicyDB(), true, RESTAPI::Errors::UnknownManagementPolicyUUID, *this ) ||
            !ValidSubscriberId(UpdateObj.subscriberId,true,Existing.primaryEmail,*this)
                ) {
            return;
        }

        ProvObjects::UpdateObjectInfo(RawObject, UserInfo_.userinfo, Existing.info);
        AssignIfPresent(RawObject,"type",Existing.type);
        AssignIfPresent(RawObject,"title",Existing.title);
        AssignIfPresent(RawObject,"salutation",Existing.salutation);
        AssignIfPresent(RawObject,"firstname",Existing.firstname);
        AssignIfPresent(RawObject,"lastname",Existing.lastname);
        AssignIfPresent(RawObject,"initials",Existing.initials);
        AssignIfPresent(RawObject,"visual",Existing.visual);
        AssignIfPresent(RawObject,"mobiles",Existing.mobiles);
        AssignIfPresent(RawObject,"phones",Existing.phones);
        AssignIfPresent(RawObject,"accessPIN",Existing.accessPIN);
        AssignIfPresent(RawObject,"secondaryEmail",Existing.secondaryEmail);

        if(DB_.UpdateRecord("id",uuid,Existing)) {
            OpContactDB::RecordName   New;
            StorageService()->OpContactDB().GetRecord("id", Existing.info.id, New);
            Poco::JSON::Object  Answer;
            New.to_json(Answer);
            return ReturnObject(Answer);
        }
        return InternalError("Contact could not be updated.");
    }
}