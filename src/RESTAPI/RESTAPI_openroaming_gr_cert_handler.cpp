//
// Created by stephane bourque on 2023-09-11.
//

#include "RESTAPI_openroaming_gr_cert_handler.h"
#include <OpenRoamin_GlobalReach.h>

namespace OpenWifi {

    void RESTAPI_openroaming_gr_cert_handler::DoGet() {
        auto Account = GetBinding("account","");
        auto Id = GetBinding("id","");

        if(Account.empty() || Id.empty()) {
            return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
        }

        if(!StorageService()->GLBLRAccountInfoDB().Exists("id",Account)) {
            return NotFound();
        }

        std::vector<ProvObjects::GLBLRCertificateInfo>  Certificates;
        DB_.GetRecords(0,1,Certificates,fmt::format(" accountId='{}' and id='{}' ", Account, Id));
        if(Certificates.empty()) {
            return NotFound();
        }
        return ReturnObject(Certificates[0]);
    }

    void RESTAPI_openroaming_gr_cert_handler::DoDelete() {
        auto Account = GetBinding("account","");
        auto Id = GetBinding("id","");
        if(Account.empty() || Id.empty()) {
            return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
        }

        if(!StorageService()->GLBLRAccountInfoDB().Exists("id",Account)) {
            return NotFound();
        }

        DB_.DeleteRecords(fmt::format(" accountId='{}' and id='{}' ", Account, Id));
        return OK();
    }

    void RESTAPI_openroaming_gr_cert_handler::DoPost() {
        auto Account = GetBinding("account","");
        auto Id = GetBinding("id","");

        if(Account.empty() || Id.empty()) {
            return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
        }

        const auto &RawObject = ParsedBody_;
        ProvObjects::GLBLRCertificateInfo   NewObject;
        if( !NewObject.from_json(RawObject)) {
            return BadRequest(OpenWifi::RESTAPI::Errors::InvalidJSONDocument);
        }

        if(NewObject.name.empty()) {
            return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
        }

        ProvObjects::GLBLRAccountInfo   AccountInfo;
        if(!StorageService()->GLBLRAccountInfoDB().GetRecord("id",Account, AccountInfo)) {
            return BadRequest(RESTAPI::Errors::InvalidGlobalReachAccount);
        }

        if(OpenRoaming_GlobalReach()->CreateRADSECCertificate(AccountInfo.GlobalReachAcctId,NewObject.name,AccountInfo.CSR, NewObject)) {
            NewObject.id = MicroServiceCreateUUID();
            NewObject.accountId = Account;
            NewObject.created = Utils::Now();
            NewObject.csr = AccountInfo.CSR;
            DB_.CreateRecord(NewObject);
            ProvObjects::GLBLRCertificateInfo   CreatedObject;
            DB_.GetRecord("id",NewObject.id,CreatedObject);
            return ReturnObject(CreatedObject);
        }

        return BadRequest(RESTAPI::Errors::RecordNotCreated);
    }

} // OpenWifi