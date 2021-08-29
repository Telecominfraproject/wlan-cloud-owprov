//
// Created by stephane bourque on 2021-08-23.
//

#include "RESTAPI_location_list_handler.h"
#include "Utils.h"
#include "RESTAPI_ProvObjects.h"
#include "StorageService.h"

namespace OpenWifi{
    void RESTAPI_location_list_handler::handleRequest(Poco::Net::HTTPServerRequest &Request,
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

    void RESTAPI_location_list_handler::DoGet(Poco::Net::HTTPServerRequest &Request,
                                             Poco::Net::HTTPServerResponse &Response) {

        try {
            std::string UUID;
            std::string Arg;

            if(!QB_.Select.empty()) {
                auto DevUUIDS = Utils::Split(QB_.Select);
                ProvObjects::LocationVec Locations;
                for(const auto &i:DevUUIDS) {
                    ProvObjects::Location E;
                    if(Storage()->LocationDB().GetRecord("id",i,E)) {
                        Locations.push_back(E);
                    } else {
                        BadRequest(Request, Response, "Unknown UUID:" + i);
                        return;
                    }
                }
                ReturnObject(Request, "locations", Locations, Response);
                return;
            } else if(QB_.CountOnly) {
                Poco::JSON::Object  Answer;
                auto C = Storage()->LocationDB().Count();
                ReturnCountOnly(Request, C, Response);
                return;
            } else {
                ProvObjects::LocationVec Locations;
                Storage()->LocationDB().GetRecords(QB_.Offset,QB_.Limit,Locations);
                ReturnObject(Request, "locations", Locations, Response);
                return;
            }
        } catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response);
    }
}