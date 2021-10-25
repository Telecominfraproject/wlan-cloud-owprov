//
// Created by stephane bourque on 2021-08-23.
//

#include "RESTAPI_location_list_handler.h"

#include "RESTObjects/RESTAPI_ProvObjects.h"
#include "StorageService.h"
#include "RESTAPI/RESTAPI_db_helpers.h"

namespace OpenWifi{

    void RESTAPI_location_list_handler::DoGet() {
        if(!QB_.Select.empty()) {
            return ReturnRecordList<decltype(StorageService()->LocationDB()),
            ProvObjects::Location>("locations",StorageService()->LocationDB(),*this );
        } else if(QB_.CountOnly) {
            Poco::JSON::Object  Answer;
            auto C = StorageService()->LocationDB().Count();
            return ReturnCountOnly(C);
        } else {
            ProvObjects::LocationVec Locations;
            StorageService()->LocationDB().GetRecords(QB_.Offset,QB_.Limit,Locations);
            return MakeJSONObjectArray("locations", Locations, *this);
        }
    }
}