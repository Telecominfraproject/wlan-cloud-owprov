//
// Created by stephane bourque on 2021-08-29.
//

#include "RESTAPI_configurations_list_handler.h"

#include "RESTObjects/RESTAPI_ProvObjects.h"
#include "StorageService.h"
#include "RESTAPI_db_helpers.h"

namespace OpenWifi{
    void RESTAPI_configurations_list_handler::DoGet() {
        if(!QB_.Select.empty()) {
            return ReturnRecordList<decltype(StorageService()->ConfigurationDB()),
                    ProvObjects::DeviceConfiguration>("configurations",StorageService()->ConfigurationDB(),*this );
        } else if(QB_.CountOnly) {
            Poco::JSON::Object  Answer;
            auto C = StorageService()->ConfigurationDB().Count();
            return ReturnCountOnly(C);
        } else {
            ProvObjects::DeviceConfigurationVec Configs;
            StorageService()->ConfigurationDB().GetRecords(QB_.Offset,QB_.Limit,Configs);
            return MakeJSONObjectArray("configurations", Configs, *this);
        }
    }
}