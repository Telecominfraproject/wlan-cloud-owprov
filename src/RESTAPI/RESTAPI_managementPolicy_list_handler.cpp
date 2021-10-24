//
// Created by stephane bourque on 2021-08-26.
//

#include "RESTAPI_managementPolicy_list_handler.h"

#include "RESTObjects/RESTAPI_ProvObjects.h"
#include "StorageService.h"
#include "framework/RESTAPI_errors.h"
#include "RESTAPI/RESTAPI_db_helpers.h"

namespace OpenWifi{
    void RESTAPI_managementPolicy_list_handler::DoGet() {
        if(!QB_.Select.empty()) {
            auto DevUUIDS = Utils::Split(QB_.Select);
            Poco::JSON::Array  ObjArr;
            for(const auto &i:DevUUIDS) {
                ProvObjects::ManagementPolicy E;
                if(StorageService()->PolicyDB().GetRecord("id",i,E)) {
                    Poco::JSON::Object  Obj;
                    E.to_json(Obj);
                    if(QB_.AdditionalInfo)
                        AddManagementPolicyExtendedInfo(E, Obj);
                    ObjArr.add(Obj);
                } else {
                    return BadRequest(RESTAPI::Errors::UnknownId + "(" + i + ")");
                }
            }
            Poco::JSON::Object  Answer;
            Answer.set("managementPolicies", ObjArr);
            return ReturnObject(Answer);
        } else if(QB_.CountOnly) {
            Poco::JSON::Object  Answer;
            auto C = StorageService()->ContactDB().Count();
            return ReturnCountOnly(C);
        } else {
            ProvObjects::ManagementPolicyVec Policies;
            StorageService()->PolicyDB().GetRecords(QB_.Offset,QB_.Limit,Policies);
            return ReturnObject("managementPolicies", Policies);
        }
    }
}