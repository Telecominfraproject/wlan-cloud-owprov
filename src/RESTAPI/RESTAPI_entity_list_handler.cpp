//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "framework/MicroService.h"
#include "RESTAPI_entity_list_handler.h"
#include "StorageService.h"
#include "RESTAPI_db_helpers.h"

namespace OpenWifi{

    void RESTAPI_entity_list_handler::DoGet() {
        if(!QB_.Select.empty()) {
            return ReturnRecordList<decltype(DB_),
            ProvObjects::Entity>("entities",DB_,*this );
        } else if(QB_.CountOnly) {
            auto C = DB_.Count();
            return ReturnCountOnly(C);
        } else if (GetBoolParameter("getTree",false)) {
            Poco::JSON::Object  FullTree;
            DB_.BuildTree(FullTree);
            return ReturnObject(FullTree);
        } else {
            EntityDB::RecordVec Entities;
            DB_.GetRecords(QB_.Offset, QB_.Limit,Entities);
            return MakeJSONObjectArray("entities", Entities, *this);
        }
    }

    void RESTAPI_entity_list_handler::DoPost() {
        if (GetBoolParameter("setTree",false)) {
            auto FullTree = ParseStream();
            DB_.ImportTree(FullTree);
            return OK();
        }
        BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
    }
}