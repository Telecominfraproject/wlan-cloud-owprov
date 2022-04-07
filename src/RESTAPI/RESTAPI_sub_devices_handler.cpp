//
// Created by stephane bourque on 2022-04-06.
//

#include "RESTAPI_sub_devices_handler.h"

namespace OpenWifi {

    void RESTAPI_sub_devices_handler::DoGet() {

        auto uuid = GetParameter("uuid");
        if(uuid.empty()) {
            return BadRequest(RESTAPI::Errors::MissingUUID);
        }

        ProvObjects::SubscriberDevice   SD;
        if(!DB_.GetRecord("id",uuid,SD)) {
            return NotFound();
        }

        Poco::JSON::Object  Answer;
        SD.to_json(Answer);
        return ReturnObject(Answer);
    }

    void RESTAPI_sub_devices_handler::DoDelete() {
    }

    void RESTAPI_sub_devices_handler::DoPost() {

    }

    void RESTAPI_sub_devices_handler::DoPut() {

    }

}