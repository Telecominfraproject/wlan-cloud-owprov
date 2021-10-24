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
            auto DevUUIDS = Utils::Split(QB_.Select);
            Poco::JSON::Array   ObjArr;
            for(const auto &i:DevUUIDS) {
                ProvObjects::ManagementRole E;
                if(StorageService()->RolesDB().GetRecord("id",i,E)) {
                    Poco::JSON::Object  Obj;
                    E.to_json(Obj);
                    if(QB_.AdditionalInfo)
                        AddManagementRoleExtendedInfo(E, Obj);
                    ObjArr.add(Obj);
                } else {
                    return BadRequest(RESTAPI::Errors::UnknownId + " (" + i + ")");
                }
            }
            Poco::JSON::Object  Answer;
            Answer.set("roles", ObjArr);
            return ReturnObject(Answer);
        } else if(QB_.CountOnly) {
            Poco::JSON::Object  Answer;
            auto C = StorageService()->RolesDB().Count();
            return ReturnCountOnly(C);
        } else {
            ProvObjects::ManagementRoleVec Roles;
            StorageService()->RolesDB().GetRecords(QB_.Offset,QB_.Limit,Roles);
            Poco::JSON::Array   ObjArr;
            for(const auto &i:Roles) {
                Poco::JSON::Object  Obj;
                i.to_json(Obj);
                if(QB_.AdditionalInfo)
                    AddManagementRoleExtendedInfo(i, Obj);
                ObjArr.add(Obj);
            }
            Poco::JSON::Object  Answer;
            Answer.set("roles", ObjArr);
            return ReturnObject(Answer);
        }
    }
}