//
// Created by stephane bourque on 2021-08-26.
//

#include "framework/MicroService.h"

#include "RESTAPI_managementRole_list_handler.h"
#include "RESTObjects/RESTAPI_ProvObjects.h"
#include "StorageService.h"
#include "framework/RESTAPI_errors.h"
#include "RESTAPI/RESTAPI_db_helpers.h"

namespace OpenWifi{
    void RESTAPI_managementRole_list_handler::DoGet() {
        if(!QB_.Select.empty()) {
            return ReturnRecordList<decltype(StorageService()->RolesDB()),
            ProvObjects::ManagementRole>("roles",StorageService()->RolesDB(),*this );
        } else if(QB_.CountOnly) {
            Poco::JSON::Object  Answer;
            auto C = StorageService()->RolesDB().Count();
            return ReturnCountOnly(C);
        } else {
            ProvObjects::ManagementRoleVec Roles;
            StorageService()->RolesDB().GetRecords(QB_.Offset,QB_.Limit,Roles);
            return MakeJSONObjectArray("roles", Roles, *this);
        }
    }
}