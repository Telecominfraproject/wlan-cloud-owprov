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

        if(RawObject->has("type")) {
            if(!NewObject.type.empty()) {
                if (ValidContactType(NewObject.type)) {
                    return BadRequest(RESTAPI::Errors::InvalidContactType);
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
            NewObject.primaryEmail = NewSubInfo.email;
        } else {
            NewObject.primaryEmail.clear();
        }

        if(RawObject->has("managementPolicy") && !StorageService()->PolicyDB().Exists("id",NewObject.managementPolicy)) {
            return BadRequest(RESTAPI::Errors::UnknownManagementPolicyUUID);
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

        if(RawObject->has("managementPolicy") && !StorageService()->PolicyDB().Exists("id",UpdateObj.managementPolicy)) {
            return BadRequest(RESTAPI::Errors::UnknownManagementPolicyUUID);
        }

        if(!UpdateObj.subscriberId.empty()) {
            SecurityObjects::UserInfo   NewSubInfo;
            if(!SDK::Sec::Subscriber::Get(this, UpdateObj.subscriberId, NewSubInfo)) {
                return BadRequest(RESTAPI::Errors::InvalidSubscriberId);
            }
            UpdateObj.primaryEmail = NewSubInfo.email;
        } else {
            UpdateObj.primaryEmail.clear();
        }

        if(RawObject->has("type")) {
            if(!UpdateObj.type.empty()) {
                if (ValidContactType(UpdateObj.type)) {
                    return BadRequest(RESTAPI::Errors::InvalidContactType);
                }
                Existing.type = Poco::toLower(UpdateObj.type);
            } else {
                Existing.type.clear();
            }
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