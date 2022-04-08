//
// Created by stephane bourque on 2022-04-07.
//

#include "RESTAPI_op_location_list_handler.h"
#include "RESTAPI_db_helpers.h"

namespace OpenWifi {
    void RESTAPI_op_location_list_handler::DoGet() {
        auto operatorId= GetParameter("operatorId");

        if(operatorId.empty() || !StorageService()->OperatorDB().Exists("id",operatorId)) {
            return BadRequest(RESTAPI::Errors::OperatorIdMustExist);
        }
        return ListHandlerForOperator<OpLocationDB>("locations", DB_, *this,operatorId);
    }

}