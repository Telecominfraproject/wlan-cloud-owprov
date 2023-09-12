//
// Created by stephane bourque on 2023-09-11.
//

#include "RESTAPI_openroaming_gr_list_certificates.h"

namespace OpenWifi {

    void RESTAPI_openroaming_gr_list_certificates::DoGet() {

        auto Account = GetParameter("account","");
        if(Account.empty()) {
            return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
        }

        auto Where = fmt::format(" accountId='{}'", Account);

        if(GetBoolParameter("countOnly")) {
            return ReturnCountOnly(DB_.Count(Where));
        }

        std::vector<ProvObjects::GLBLRCertificateInfo>  Certificates;
        DB_.GetRecords(QB_.Offset,QB_.Limit,Certificates, Where);
        return ReturnObject(Certificates);
    }

} // OpenWifi