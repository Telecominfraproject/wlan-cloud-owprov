//
// Created by stephane bourque on 2021-11-09.
//

#include "RESTAPI_map_list_handler.h"
#include "RESTObjects/RESTAPI_ProvObjects.h"
#include "StorageService.h"
#include "RESTAPI/RESTAPI_db_helpers.h"

namespace OpenWifi{
    void RESTAPI_map_list_handler::DoGet() {
        if(GetBoolParameter("myMaps",false)) {
            auto where = DB_.OP("creator",ORM::EQ,UserInfo_.userinfo.id);
            std::vector<ProvObjects::Map>   Maps;
            DB_.GetRecords(QB_.Offset,QB_.Limit,Maps,where);
            return MakeJSONObjectArray("list", Maps, *this);
        } else if(GetBoolParameter("sharedWithMe",false)) {

        } else if(!QB_.Select.empty()) {
            return ReturnRecordList<decltype(DB_),ProvObjects::Map>("list",DB_,*this );
        } else if(QB_.CountOnly) {
            Poco::JSON::Object  Answer;
            auto C = DB_.Count();
            return ReturnCountOnly(C);
        } else {
            MapDB::RecordVec Maps;
            DB_.GetRecords(QB_.Offset,QB_.Limit,Maps);
            return MakeJSONObjectArray("list", Maps, *this);
        }
    }
}