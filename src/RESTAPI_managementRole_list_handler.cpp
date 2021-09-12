//
// Created by stephane bourque on 2021-08-26.
//

#include "RESTAPI_managementRole_list_handler.h"

#include "Utils.h"
#include "RESTAPI_ProvObjects.h"
#include "StorageService.h"

namespace OpenWifi{
    void RESTAPI_managementRole_list_handler::DoGet() {
        try {
            if(!QB_.Select.empty()) {
                auto DevUUIDS = Utils::Split(QB_.Select);
                ProvObjects::ManagementRoleVec Roles;
                for(const auto &i:DevUUIDS) {
                    ProvObjects::ManagementRole E;
                    if(Storage()->RolesDB().GetRecord("id",i,E)) {
                        Roles.push_back(E);
                    } else {
                        BadRequest("Unknown UUID:" + i);
                        return;
                    }
                }
                ReturnObject("roles", Roles);
                return;
            } else if(QB_.CountOnly) {
                Poco::JSON::Object  Answer;
                auto C = Storage()->RolesDB().Count();
                ReturnCountOnly(C);
                return;
            } else {
                ProvObjects::ManagementRoleVec Roles;
                Storage()->RolesDB().GetRecords(QB_.Offset,QB_.Limit,Roles);
                ReturnObject("roles", Roles);
                return;
            }
        } catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest("Internal error.");
    }

    void RESTAPI_managementRole_list_handler::DoDelete() {}
    void RESTAPI_managementRole_list_handler::DoPut() {}
    void RESTAPI_managementRole_list_handler::DoPost() {}
}