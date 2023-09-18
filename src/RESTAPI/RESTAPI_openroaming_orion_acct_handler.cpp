//
// Created by stephane bourque on 2023-09-15.
//

#include "RESTAPI_openroaming_orion_acct_handler.h"
#include "OpenRoamin_GlobalReach.h"

namespace OpenWifi {

    void RESTAPI_openroaming_orion_acct_handler::DoGet() {
        auto Account = GetBinding("id","");
        if(Account.empty()) {
            return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
        }

        ProvObjects::GooglOrionAccountInfo   Record;
        if(DB_.GetRecord("id",Account,Record)) {
            return ReturnObject(Record);
        }
        return NotFound();
    }

    void RESTAPI_openroaming_orion_acct_handler::DoDelete() {
        auto Account = GetBinding("id","");
        if(Account.empty()) {
            return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
        }

        ProvObjects::GooglOrionAccountInfo   Record;
        if(!DB_.GetRecord("id",Account,Record)) {
            return NotFound();
        }
        DB_.DeleteRecord("id", Account);
        return OK();
    }

    void RESTAPI_openroaming_orion_acct_handler::DoPost() {
        auto Account = GetBinding("id","");
        if(Account.empty()) {
            return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
        }

        const auto &RawObject = ParsedBody_;
        ProvObjects::GooglOrionAccountInfo    NewObject;
        if( !NewObject.from_json(RawObject)) {
            return BadRequest(OpenWifi::RESTAPI::Errors::InvalidJSONDocument);
        }

        if( NewObject.privateKey.empty()    ||
            NewObject.certificate.empty()   ||
            NewObject.cacerts.empty()) {
            return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
        }

        if( !Utils::VerifyRSAKey(NewObject.privateKey)           ||
            !Utils::ValidX509Certificate(NewObject.certificate) ||
            !Utils::ValidX509Certificate(NewObject.cacerts)) {
            return BadRequest(RESTAPI::Errors::NotAValidECKey);
        }

        ProvObjects::CreateObjectInfo(RawObject,UserInfo_.userinfo,NewObject.info);

        if(DB_.CreateRecord(NewObject)) {
            ProvObjects::GooglOrionAccountInfo StoredObject;
            DB_.GetRecord("id",NewObject.info.id,StoredObject);
            return ReturnObject(StoredObject);
        }
        return BadRequest(RESTAPI::Errors::RecordNotCreated);
    }

    void RESTAPI_openroaming_orion_acct_handler::DoPut() {
        auto Account = GetBinding("account","");
        if(Account.empty()) {
            return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
        }

        const auto &RawObject = ParsedBody_;
        ProvObjects::GLBLRAccountInfo    Modify;
        if(!Modify.from_json(RawObject)) {
            return BadRequest(OpenWifi::RESTAPI::Errors::InvalidJSONDocument);
        }

        ProvObjects::GooglOrionAccountInfo    Existing;
        if(!DB_.GetRecord("id",Account,Existing)) {
            return NotFound();
        }

        if(!ProvObjects::UpdateObjectInfo(RawObject,UserInfo_.userinfo,Existing.info)) {
            return BadRequest(OpenWifi::RESTAPI::Errors::InvalidJSONDocument);
        }

        if(DB_.UpdateRecord("id",Existing.info.id,Existing)) {
            ProvObjects::GooglOrionAccountInfo StoredObject;
            DB_.GetRecord("id",Existing.info.id,StoredObject);
            return ReturnObject(StoredObject);
        }
        return BadRequest(RESTAPI::Errors::RecordNotUpdated);
    }

} // OpenWifi