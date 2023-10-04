//
// Created by stephane bourque on 2023-09-27.
//

#include "RESTAPI_radiusendpoint_list_handler.h"
#include "framework/AppServiceRegistry.h"
#include "RadiusEndpointUpdater.h"

namespace OpenWifi {

    void RESTAPI_radiusendpoint_list_handler::DoGet() {
        if(GetBoolParameter("lastUpdate")) {
            uint64_t LastUpdate=0;
            AppServiceRegistry().Get("radiusEndpointLastUpdate", LastUpdate);
            Poco::JSON::Object  Answer;
            Answer.set("lastUpdate",LastUpdate);
            return ReturnObject(Answer);
        }

        if(QB_.CountOnly) {
            return ReturnCountOnly(DB_.Count());
        }
        std::vector<RecordType>    Records;
        DB_.GetRecords(QB_.Offset,QB_.Limit,Records);
        return ReturnObject(Records);
    }

    void RESTAPI_radiusendpoint_list_handler::DoPut() {
        if( UserInfo_.userinfo.userRole!=SecurityObjects::ROOT &&
            UserInfo_.userinfo.userRole!=SecurityObjects::ADMIN) {
            return BadRequest(RESTAPI::Errors::ACCESS_DENIED);
        }

        if(GetBoolParameter("updateEndpoints")) {
            RadiusEndpointUpdater R;

            std::string Error;
            uint64_t ErrorNum = 0;
            R.UpdateEndpoints(this, Error, ErrorNum);

            Poco::JSON::Object  Answer;
            Answer.set("Error", Error);
            Answer.set("ErrorNum", ErrorNum);
            return ReturnObject(Answer);
        }
        return BadRequest(RESTAPI::Errors::MissingAuthenticationInformation);
    }

} // OpenWifi