//
// Created by stephane bourque on 2023-09-11.
//

#include "RESTAPI_openroaming_gr_list_acct_handler.h"

namespace OpenWifi {

    void RESTAPI_openroaming_gr_list_acct_handler::DoGet() {

        if(GetBoolParameter("countOnly")) {
            return ReturnCountOnly(DB_.Count());
        }

        std::vector<ProvObjects::GLBLRAccountInfo>  Accounts;
        DB_.GetRecords(QB_.Offset,QB_.Limit,Accounts);
        return ReturnObject(Accounts);
    }

} // OpenWifi