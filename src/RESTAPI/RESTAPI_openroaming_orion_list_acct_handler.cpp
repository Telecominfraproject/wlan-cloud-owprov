//
// Created by stephane bourque on 2023-09-15.
//

#include "RESTAPI_openroaming_orion_list_acct_handler.h"


namespace OpenWifi {

    void RESTAPI_openroaming_orion_list_acct_handler::DoGet() {

        if(GetBoolParameter("countOnly")) {
            return ReturnCountOnly(DB_.Count());
        }

        std::vector<ProvObjects::GooglOrionAccountInfo>  Accounts;
        DB_.GetRecords(QB_.Offset,QB_.Limit,Accounts);
        return ReturnObject(Accounts);
    }

} // OpenWifi