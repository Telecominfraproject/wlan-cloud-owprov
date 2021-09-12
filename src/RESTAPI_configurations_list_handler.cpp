//
// Created by stephane bourque on 2021-08-29.
//

#include "RESTAPI_configurations_list_handler.h"

#include "Utils.h"
#include "RESTAPI_ProvObjects.h"
#include "StorageService.h"
#include "RESTAPI_errors.h"

namespace OpenWifi{
    void RESTAPI_configurations_list_handler::DoGet() {
        if(!QB_.Select.empty()) {
            auto DevUUIDS = Utils::Split(QB_.Select);
            ProvObjects::DeviceConfigurationVec Configs;
            for(const auto &i:DevUUIDS) {
                ProvObjects::DeviceConfiguration E;
                if(Storage()->ConfigurationDB().GetRecord("id",i,E)) {
                    Configs.push_back(E);
                } else {
                    BadRequest(RESTAPI::Errors::UnknownId + " (" + i + ")");
                    return;
                }
            }
            ReturnObject("configurations", Configs);
            return;
        } else if(QB_.CountOnly) {
            Poco::JSON::Object  Answer;
            auto C = Storage()->ConfigurationDB().Count();
            ReturnCountOnly(C);
            return;
        } else {
            ProvObjects::DeviceConfigurationVec Configs;
            Storage()->ConfigurationDB().GetRecords(QB_.Offset,QB_.Limit,Configs);
            ReturnObject("configurations", Configs);
            return;
        }
    }
}