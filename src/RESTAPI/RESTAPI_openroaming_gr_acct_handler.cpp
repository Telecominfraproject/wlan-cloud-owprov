//
// Created by stephane bourque on 2023-09-11.
//

#include "RESTAPI_openroaming_gr_acct_handler.h"
#include "OpenRoamin_GlobalReach.h"

namespace OpenWifi {

    void RESTAPI_openroaming_gr_acct_handler::DoGet() {
        auto Account = GetBinding("account","");
        if(Account.empty()) {
            return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
        }

        ProvObjects::GLBLRAccountInfo   Record;
        if(DB_.GetRecord("id",Account,Record)) {
            return ReturnObject(Record);
        }
        return NotFound();
    }

    void RESTAPI_openroaming_gr_acct_handler::DoDelete() {
        auto Account = GetBinding("account","");
        if(Account.empty()) {
            return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
        }

        ProvObjects::GLBLRAccountInfo   Record;
        if(!DB_.GetRecord("id",Account,Record)) {
            return NotFound();
        }

        StorageService()->GLBLRCertsDB().DeleteRecords(fmt::format(" accountId='{}' ", Account));
        DB_.DeleteRecord("id", Account);

        return OK();
    }

    void RESTAPI_openroaming_gr_acct_handler::DoPost() {
        auto Account = GetBinding("account","");
        if(Account.empty()) {
            return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
        }

        const auto &RawObject = ParsedBody_;
        ProvObjects::GLBLRAccountInfo    NewObject;
        if( !NewObject.from_json(RawObject)) {
            return BadRequest(OpenWifi::RESTAPI::Errors::InvalidJSONDocument);
        }

        if(NewObject.privateKey.empty() || NewObject.GlobalReachAcctId.empty()) {
            return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
        }

        if(!NewObject.privateKey.empty() && !Utils::VerifyECKey(NewObject.privateKey)) {
            return BadRequest(RESTAPI::Errors::NotAValidECKey);
        }

        std::string GlobalReachName;
        if(!OpenRoaming_GlobalReach()->VerifyAccount(NewObject.GlobalReachAcctId,NewObject.privateKey,GlobalReachName)) {
            return BadRequest(RESTAPI::Errors::InvalidGlobalReachAccount);
        }

        if( NewObject.commonName.empty() || NewObject.organization.empty() ||
            NewObject.city.empty() || NewObject.province.empty() || NewObject.country.empty() ) {
            return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
        }

        Utils::CSRCreationParameters    P;
        P.Country = NewObject.country;
        P.CommonName = NewObject.commonName;
        P.Province = NewObject.province;
        P.City = NewObject.city;
        P.Organization = NewObject.organization;
        Utils::CSRCreationResults       R;
        if(!Utils::CreateX509CSR(P,R)) {
            return BadRequest(RESTAPI::Errors::CannotCreateCSR);
        }

        ProvObjects::CreateObjectInfo(RawObject,UserInfo_.userinfo,NewObject.info);

        if(DB_.CreateRecord(NewObject)) {
            ProvObjects::GLBLRAccountInfo StoredObject;
            DB_.GetRecord("id",NewObject.info.id,StoredObject);
            return ReturnObject(StoredObject);
        }

        return BadRequest(RESTAPI::Errors::RecordNotCreated);
    }

    void RESTAPI_openroaming_gr_acct_handler::DoPut() {
        auto Account = GetBinding("account","");
        if(Account.empty()) {
            return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
        }

        const auto &RawObject = ParsedBody_;
        ProvObjects::GLBLRAccountInfo    Modify;
        if(!Modify.from_json(RawObject)) {
            return BadRequest(OpenWifi::RESTAPI::Errors::InvalidJSONDocument);
        }

        ProvObjects::GLBLRAccountInfo    Existing;
        if(!DB_.GetRecord("id",Account,Existing)) {
            return NotFound();
        }

        if(!ProvObjects::UpdateObjectInfo(RawObject,UserInfo_.userinfo,Existing.info)) {
            return BadRequest(OpenWifi::RESTAPI::Errors::InvalidJSONDocument);
        }

        if(DB_.UpdateRecord("id",Existing.info.id,Existing)) {
            ProvObjects::GLBLRAccountInfo StoredObject;
            DB_.GetRecord("id",Existing.info.id,StoredObject);
            return ReturnObject(StoredObject);
        }
        return BadRequest(RESTAPI::Errors::RecordNotUpdated);
    }

} // OpenWifi