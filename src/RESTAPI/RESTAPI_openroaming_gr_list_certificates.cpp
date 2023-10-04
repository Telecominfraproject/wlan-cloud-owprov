//
// Created by stephane bourque on 2023-09-11.
//

#include "RESTAPI_openroaming_gr_list_certificates.h"

namespace OpenWifi {

    void RESTAPI_openroaming_gr_list_certificates::DoGet() {
        auto Account = GetBinding("account");
        if(Account.empty()) {
            return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
        }

        if(Account=="*") {
            std::vector< ProvObjects::GLBLRCertificateInfo> Arr;
            for(const auto &cert:QB_.Select) {
                ProvObjects::GLBLRCertificateInfo CInfo;
                if(StorageService()->GLBLRCertsDB().GetRecord("id",cert,CInfo)) {
                    Arr.emplace_back(CInfo);
                }
            }
            return ReturnObject(Arr);
        }

        auto Where = fmt::format(" accountId='{}'", Account);
        if(GetBoolParameter("countOnly")) {
            return ReturnCountOnly(DB_.Count(Where));
        }

        std::vector<RecordType>  Certificates;
        DB_.GetRecords(QB_.Offset,QB_.Limit,Certificates, Where);
        return ReturnObject(Certificates);
    }

} // OpenWifi