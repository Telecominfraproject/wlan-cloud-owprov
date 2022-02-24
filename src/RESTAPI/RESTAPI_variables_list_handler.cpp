//
// Created by stephane bourque on 2022-02-23.
//

#include "RESTAPI_variables_list_handler.h"
#include "RESTAPI/RESTAPI_db_helpers.h"

namespace OpenWifi {

    void RESTAPI_variables_list_handler::DoGet() {
        if(!QB_.Select.empty()) {
            return ReturnRecordList<decltype(DB_),
                    ProvObjects::VariableBlock>("variableBlocks",DB_,*this );
        } else if(QB_.CountOnly) {
            Poco::JSON::Object  Answer;
            auto C = DB_.Count();
            return ReturnCountOnly(C);
        } else {
            VariablesDB::RecordVec VariableBlocks;
            DB_.GetRecords(QB_.Offset,QB_.Limit,VariableBlocks);
            return MakeJSONObjectArray("variableBlocks", VariableBlocks, *this);
        }
    }

}