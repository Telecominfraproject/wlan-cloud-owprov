//
// Created by stephane bourque on 2021-08-29.
//

#include "RESTAPI_configurations_list_handler.h"

#include "Utils.h"
#include "RESTAPI_ProvObjects.h"
#include "StorageService.h"

namespace OpenWifi{
    void RESTAPI_configurations_list_handler::handleRequest(Poco::Net::HTTPServerRequest &Request,
                                                     Poco::Net::HTTPServerResponse &Response) {
        if (!ContinueProcessing(Request, Response))
            return;

        if (!IsAuthorized(Request, Response))
            return;

        ParseParameters(Request);
        if(Request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET)
            DoGet(Request, Response);
        else
            BadRequest(Request, Response, "Unknown HTTP Method");
    }

    void RESTAPI_configurations_list_handler::DoGet(Poco::Net::HTTPServerRequest &Request,
                                             Poco::Net::HTTPServerResponse &Response) {
        try {
            if(!QB_.Select.empty()) {
                auto DevUUIDS = Utils::Split(QB_.Select);
                ProvObjects::DeviceConfigurationVec Configs;
                for(const auto &i:DevUUIDS) {
                    ProvObjects::DeviceConfiguration E;
                    if(Storage()->ConfigurationDB().GetRecord("id",i,E)) {
                        Configs.push_back(E);
                    } else {
                        BadRequest(Request, Response, "Unknown UUID:" + i);
                        return;
                    }
                }
                ReturnObject(Request, "configurations", Configs, Response);
                return;
            } else if(QB_.CountOnly) {
                Poco::JSON::Object  Answer;
                auto C = Storage()->ConfigurationDB().Count();
                ReturnCountOnly(Request,C,Response);
                return;
            } else {
                ProvObjects::DeviceConfigurationVec Configs;
                Storage()->ConfigurationDB().GetRecords(QB_.Offset,QB_.Limit,Configs);
                ReturnObject(Request, "configurations", Configs, Response);
                return;
            }
        } catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response);
    }
}