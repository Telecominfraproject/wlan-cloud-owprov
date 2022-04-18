//
// Created by stephane bourque on 2022-04-06.
//

#include "RESTAPI_operators_handler.h"
#include "RESTAPI_db_helpers.h"

namespace OpenWifi {

    void RESTAPI_operators_handler::DoGet() {
        auto uuid = GetBinding("uuid","");
        if(uuid.empty()) {
            return BadRequest(RESTAPI::Errors::MissingUUID);
        }

        OperatorDB::RecordName  Existing;
        if(!DB_.GetRecord("id",uuid,Existing)) {
            return NotFound();
        }

        Poco::JSON::Object  Answer;
        Existing.to_json(Answer);
        return ReturnObject(Answer);
    }

    void RESTAPI_operators_handler::DoDelete() {
        auto uuid = GetBinding("uuid","");
        if(uuid.empty()) {
            return BadRequest(RESTAPI::Errors::MissingUUID);
        }

        OperatorDB::RecordName  Existing;
        if(!DB_.GetRecord("id",uuid,Existing)) {
            return NotFound();
        }

        if(Existing.defaultOperator) {
            return BadRequest(RESTAPI::Errors::CannotDeleteDefaultOperator);
        }

        //  Let's see if there are any subscribers in this operator
        auto Count = StorageService()->SubscriberDeviceDB().Count(fmt::format(" operatorId='{}'", uuid));
        if(Count>0) {
            return BadRequest(RESTAPI::Errors::StillInUse);
        }

        DB_.DeleteRecord("id",uuid);
        StorageService()->ServiceClassDB().DeleteRecords(fmt::format(" operatorId='{}'", uuid));
        return OK();
    }

    void RESTAPI_operators_handler::DoPost() {

        auto RawObject = ParseStream();
        ProvObjects::Operator   NewObject;
        if(!NewObject.from_json(RawObject)) {
            return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
        }

        if(NewObject.defaultOperator) {
            return BadRequest(RESTAPI::Errors::CannotCreateDefaultOperator);
        }

        if(RawObject->has("managementPolicy") && !StorageService()->PolicyDB().Exists("id",NewObject.managementPolicy)) {
            return BadRequest(RESTAPI::Errors::UnknownManagementPolicyUUID);
        }

        if(NewObject.rrm.empty())
            NewObject.rrm = "inherit";

        if(!ValidRRM(NewObject.rrm)) {
            return BadRequest(RESTAPI::Errors::InvalidRRM);
        }

        if(!ValidSourceIP(NewObject.sourceIP)) {
            return BadRequest(RESTAPI::Errors::InvalidIPAddresses);
        }

        ProvObjects::CreateObjectInfo(RawObject, UserInfo_.userinfo, NewObject.info);
        if(DB_.CreateRecord(NewObject)) {

            // Create the default service...

            ProvObjects::ServiceClass DefSer;
            DefSer.info.id = MicroService::CreateUUID();
            DefSer.info.name = "Default Service Class";
            DefSer.defaultService = true;
            DefSer.info.created = DefSer.info.modified = OpenWifi::Now();
            DefSer.operatorId = NewObject.info.id;
            DefSer.period = "monthly";
            DefSer.billingCode = "basic";
            DefSer.currency = "USD";
            StorageService()->ServiceClassDB().CreateRecord(DefSer);

            ProvObjects::Operator   New;
            DB_.GetRecord("id",NewObject.info.id,New);
            Poco::JSON::Object  Answer;
            New.to_json(Answer);
            return ReturnObject(Answer);
        }

        return InternalError("Failed creating operator.");
    }

    void RESTAPI_operators_handler::DoPut() {
        auto uuid = GetBinding("uuid","");
        if(uuid.empty()) {
            return BadRequest(RESTAPI::Errors::MissingUUID);
        }

        ProvObjects::Operator   Existing;
        if(!DB_.GetRecord("id",uuid,Existing)) {
            return NotFound();
        }

        auto RawObject = ParseStream();
        ProvObjects::Operator   UpdatedObj;
        if(!UpdatedObj.from_json(RawObject)) {
            return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
        }

        if(RawObject->has("managementPolicy")) {
            if(!StorageService()->PolicyDB().Exists("id",UpdatedObj.managementPolicy)) {
                return BadRequest(RESTAPI::Errors::UnknownManagementPolicyUUID);
            }
            Existing.managementPolicy = UpdatedObj.managementPolicy;
        }

        ProvObjects::UpdateObjectInfo(RawObject, UserInfo_.userinfo, Existing.info);

        if(RawObject->has("variables")) {
            Existing.variables = UpdatedObj.variables;
        }

        if(RawObject->has("sourceIP")) {
            if(!UpdatedObj.sourceIP.empty() && !ValidSourceIP(UpdatedObj.sourceIP)) {
                return BadRequest(RESTAPI::Errors::InvalidIPAddresses);
            }
            Existing.sourceIP = UpdatedObj.sourceIP;
        }

        if(RawObject->has("rrm")) {
            if(!UpdatedObj.rrm.empty() && !ValidRRM(UpdatedObj.rrm)) {
                return BadRequest(RESTAPI::Errors::InvalidRRM);
            }
            Existing.rrm = UpdatedObj.rrm.empty() ? "inherit" : UpdatedObj.rrm;
        }

        AssignIfPresent(RawObject, "firmwareUpgrade", Existing.firmwareUpgrade);
        AssignIfPresent(RawObject, "firmwareRCOnly", Existing.firmwareRCOnly);

        return ReturnUpdatedObject(DB_, Existing, *this);
    }

}