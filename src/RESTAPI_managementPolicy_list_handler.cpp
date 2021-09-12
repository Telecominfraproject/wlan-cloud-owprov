//
// Created by stephane bourque on 2021-08-26.
//

#include "RESTAPI_managementPolicy_list_handler.h"

#include "Utils.h"
#include "RESTAPI_ProvObjects.h"
#include "StorageService.h"
#include "RESTAPI_errors.h"

namespace OpenWifi{
    void RESTAPI_managementPolicy_list_handler::DoGet() {
        if(!QB_.Select.empty()) {
            auto DevUUIDS = Utils::Split(QB_.Select);
            ProvObjects::ManagementPolicyVec Policies;
            for(const auto &i:DevUUIDS) {
                ProvObjects::ManagementPolicy E;
                if(Storage()->PolicyDB().GetRecord("id",i,E)) {
                    Policies.push_back(E);
                } else {
                    BadRequest(RESTAPI::Errors::UnknownId + "(" + i + ")");
                    return;
                }
            }
            ReturnObject("managementPolicies", Policies);
            return;
        } else if(QB_.CountOnly) {
            Poco::JSON::Object  Answer;
            auto C = Storage()->ContactDB().Count();
            ReturnCountOnly(C);
            return;
        } else {
            ProvObjects::ManagementPolicyVec Policies;
            Storage()->PolicyDB().GetRecords(QB_.Offset,QB_.Limit,Policies);
            ReturnObject("managementPolicies", Policies);
            return;
        }
    }

}