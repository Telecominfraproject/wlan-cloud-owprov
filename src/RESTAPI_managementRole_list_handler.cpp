//
// Created by stephane bourque on 2021-08-26.
//

#include "RESTAPI_managementRole_list_handler.h"

#include "Utils.h"
#include "RESTAPI_ProvObjects.h"
#include "StorageService.h"
#include "RESTAPI_errors.h"

namespace OpenWifi{
    void RESTAPI_managementRole_list_handler::DoGet() {
        if(!QB_.Select.empty()) {
            auto DevUUIDS = Utils::Split(QB_.Select);
            ProvObjects::ManagementRoleVec Roles;
            for(const auto &i:DevUUIDS) {
                ProvObjects::ManagementRole E;
                if(Storage()->RolesDB().GetRecord("id",i,E)) {
                    Roles.push_back(E);
                } else {
                    return BadRequest(RESTAPI::Errors::UnknownId + " (" + i + ")");
                }
            }
            return ReturnObject("roles", Roles);
        } else if(QB_.CountOnly) {
            Poco::JSON::Object  Answer;
            auto C = Storage()->RolesDB().Count();
            return ReturnCountOnly(C);
        } else {
            ProvObjects::ManagementRoleVec Roles;
            Storage()->RolesDB().GetRecords(QB_.Offset,QB_.Limit,Roles);
            return ReturnObject("roles", Roles);
        }
    }
}