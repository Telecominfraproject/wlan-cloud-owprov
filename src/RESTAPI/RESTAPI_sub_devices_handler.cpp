//
// Created by stephane bourque on 2022-04-06.
//

#include "RESTAPI_sub_devices_handler.h"
#include "RESTAPI/RESTAPI_db_helpers.h"
#include "sdks/SDK_sec.h"

namespace OpenWifi {

    void RESTAPI_sub_devices_handler::DoGet() {
        auto uuid = GetBinding("uuid");
        if(uuid.empty()) {
            return BadRequest(RESTAPI::Errors::MissingUUID);
        }

        ProvObjects::SubscriberDevice   SD;
        if(!DB_.GetRecord("id",uuid,SD)) {
            return NotFound();
        }

        Poco::JSON::Object  Answer;
        SD.to_json(Answer);
        return ReturnObject(Answer);
    }

    void RESTAPI_sub_devices_handler::DoDelete() {
        auto uuid = GetBinding("uuid");
        if(uuid.empty()) {
            return BadRequest(RESTAPI::Errors::MissingUUID);
        }

        if(!DB_.Exists("id",uuid)) {
            return NotFound();
        }

        DB_.DeleteRecord("id",uuid);
        return OK();
    }

    void RESTAPI_sub_devices_handler::DoPost() {

        auto RawObject = ParseStream();
        SubscriberDeviceDB::RecordName NewObject;
        if(!NewObject.from_json(RawObject)) {
            return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
        }

        if( !ValidDbId(NewObject.managementPolicy, StorageService()->PolicyDB(), true , RESTAPI::Errors::UnknownManagementPolicyUUID, *this) ||
            !ValidDbId(NewObject.contact, StorageService()->OpContactDB(), true, RESTAPI::Errors::InvalidContactId, *this)                   ||
            !ValidDbId(NewObject.location, StorageService()->OpLocationDB(), true, RESTAPI::Errors::InvalidLocationId, *this)                ||
            !ValidDbId(NewObject.operatorId, StorageService()->OperatorDB(), true, RESTAPI::Errors::InvalidOperatorId, *this)                ||
            !ValidDbId(NewObject.serviceClass, StorageService()->ServiceClassDB(), true, RESTAPI::Errors::InvalidServiceClassId, *this)      ||
            !ValidSubscriberId(NewObject.subscriberId, true, *this) ||
            !ValidRRM(NewObject.rrm,*this) ||
            !ValidSerialNumber(NewObject.serialNumber,false,*this)
                ) {
            return;
        }

        ProvObjects::CreateObjectInfo(RawObject,UserInfo_.userinfo,NewObject.info);
        if(DB_.CreateRecord(NewObject)) {
            SubscriberDeviceDB::RecordName New;
            DB_.GetRecord("id",NewObject.info.id,New);
            Poco::JSON::Object  Answer;
            New.to_json(Answer);
            return ReturnObject(Answer);
        }
        return InternalError("Count not create record.");

    }

    void RESTAPI_sub_devices_handler::DoPut() {
        auto uuid = GetBinding("uuid");

        auto RawObject = ParseStream();
        SubscriberDeviceDB::RecordName UpdateObj;
        if(!UpdateObj.from_json(RawObject)) {
            return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
        }

        SubscriberDeviceDB::RecordName  Existing;
        if(!DB_.GetRecord("id",uuid,Existing)) {
            return NotFound();
        }


    }

}