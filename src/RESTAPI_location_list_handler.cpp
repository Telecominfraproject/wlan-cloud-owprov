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
                Poco::JSON::Array   Arr;
                for(const auto &i:DevUUIDS) {
                    ProvObjects::Location E;
                    if(Storage()->LocationDB().GetRecord("id",i,E)) {
                        Poco::JSON::Object  O;
                        E.to_json(O);
                        Arr.add(O);
                    } else {
                        BadRequest(Request, Response, "Unknown UUID:" + i);
                        return;
                    }
                }
                Poco::JSON::Object  Answer;
                Answer.set("locations",Arr);
                ReturnObject(Request, Answer, Response);
                return;
            } else if(HasParameter("countOnly",Arg) && Arg=="true") {
                Poco::JSON::Object  Answer;
                auto C = Storage()->LocationDB().Count();
                Answer.set("count", C);
                ReturnObject(Request, Answer, Response);
                return;
            } else {
                LocationVec Locations;
                Storage()->LocationDB().GetRecords(QB_.Offset,QB_.Limit,Locations);
                Poco::JSON::Array   Arr;
                for(const auto &i:Locations) {
                    Poco::JSON::Object  O;
                    i.to_json(O);
                    Arr.add(O);
                }
                Poco::JSON::Object  Answer;
                Answer.set("locations",Arr);
                ReturnObject(Request, Answer, Response);
                return;
            }
        } catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response);
    }
}