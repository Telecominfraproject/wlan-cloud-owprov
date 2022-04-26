//
// Created by stephane bourque on 2022-04-06.
//

#include "RESTAPI_sub_devices_list_handler.h"
#include "RESTAPI/RESTAPI_db_helpers.h"


namespace OpenWifi {

    void RESTAPI_sub_devices_list_handler::DoGet() {
        auto operatorId=GetParameter("operatorId");
        auto subscriberId=GetParameter("subscriberId");

        if(!operatorId.empty() && !StorageService()->OperatorDB().Exists("id",operatorId)) {
            return BadRequest(RESTAPI::Errors::OperatorIdMustExist);
        }

        return ListHandlerForOperator<SubscriberDeviceDB>("subscriberDevices", DB_, *this, operatorId, subscriberId);
    }

}