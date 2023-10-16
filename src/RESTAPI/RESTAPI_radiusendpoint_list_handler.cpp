//
// Created by stephane bourque on 2023-09-27.
//

#include "RESTAPI_radiusendpoint_list_handler.h"
#include "framework/AppServiceRegistry.h"
#include "RadiusEndpointUpdater.h"

namespace OpenWifi {

    void RESTAPI_radiusendpoint_list_handler::DoGet() {

        if(GetBoolParameter("currentStatus")) {
            ProvObjects::RADIUSEndpointUpdateStatus Status;
            Status.Read();
            return ReturnObject(Status);
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

            std::uint64_t ErrorCode;
            std::string ErrorDetails;
            std::string ErrorDescription;

            if(!R.UpdateEndpoints(this, ErrorCode, ErrorDetails,ErrorDescription)) {
                return InternalError(RESTAPI::Errors::msg{.err_num = ErrorCode, .err_txt = ErrorDetails + ":" + ErrorDescription});
            }
            return OK();
        }
        return BadRequest(RESTAPI::Errors::MissingAuthenticationInformation);
    }

} // OpenWifi