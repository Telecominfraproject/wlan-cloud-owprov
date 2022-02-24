//
// Created by stephane bourque on 2021-08-26.
//

#include "framework/MicroService.h"

#include "RESTAPI_managementRole_list_handler.h"
#include "RESTObjects/RESTAPI_ProvObjects.h"
#include "StorageService.h"
#include "RESTAPI/RESTAPI_db_helpers.h"

namespace OpenWifi{
    void RESTAPI_managementRole_list_handler::DoGet() {
        if(!QB_.Select.empty()) {
            return ReturnRecordList<decltype(DB_),
            ProvObjects::ManagementRole>("roles",DB_,*this );
        } else if(QB_.CountOnly) {
            Poco::JSON::Object  Answer;
            auto C = DB_.Count();
            return ReturnCountOnly(C);
        } else {
            ManagementRoleDB::RecordVec Roles;
            DB_.GetRecords(QB_.Offset,QB_.Limit,Roles);
            return MakeJSONObjectArray("roles", Roles, *this);
        }
    }
}