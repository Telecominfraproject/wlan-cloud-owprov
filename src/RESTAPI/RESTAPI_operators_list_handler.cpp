//
// Created by stephane bourque on 2022-04-06.
//

#include "RESTAPI_operators_list_handler.h"
#include "RESTAPI/RESTAPI_db_helpers.h"


namespace OpenWifi {
    void RESTAPI_operators_list_handler::DoGet() {

            if(QB_.CountOnly) {
                auto Count = DB_.Count();
                return ReturnCountOnly(Count);
            }

            if(!QB_.Select.empty()) {
                return ReturnRecordList<decltype(DB_), ProvObjects::Operator>("operators", DB_, *this);
            }

            std::vector<ProvObjects::Operator>  Entries;
            DB_.GetRecords(QB_.Offset,QB_.Limit,Entries);
            return MakeJSONObjectArray("operators", Entries, *this);
    }
}