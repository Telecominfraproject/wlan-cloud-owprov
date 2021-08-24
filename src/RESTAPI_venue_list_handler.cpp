//
// Created by stephane bourque on 2021-08-23.
//

#include "RESTAPI_venue_list_handler.h"
#include "Utils.h"
#include "StorageService.h"

namespace OpenWifi{
    void RESTAPI_venue_list_handler::handleRequest(Poco::Net::HTTPServerRequest &Request,
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

    void RESTAPI_venue_list_handler::DoGet(Poco::Net::HTTPServerRequest &Request,
                                            Poco::Net::HTTPServerResponse &Response) {
        try {
            std::string Arg;
            if(!QB_.Select.empty()) {
                auto UUIDs = Utils::Split(QB_.Select);
                Poco::JSON::Array   Arr;
                for(const auto &i:UUIDs) {
                    ProvObjects::Venue E;
                    if(Storage()->VenueDB().GetRecord("id",i,E)) {
                        Poco::JSON::Object  O;
                        E.to_json(O);
                        Arr.add(O);
                    } else {
                        BadRequest(Request, Response, "Unknown UUID:" + i);
                        return;
                    }
                }
                Poco::JSON::Object  Answer;
                Answer.set("venues",Arr);
                ReturnObject(Request, Answer, Response);
                return;
            } else if(HasParameter("countOnly",Arg) && Arg=="true") {
                Poco::JSON::Object  Answer;
                auto C = Storage()->VenueDB().Count();
                Answer.set("count", C);
                ReturnObject(Request, Answer, Response);
                return;
            } else {
                VenueVec Venues;
                Storage()->VenueDB().GetRecords(QB_.Offset, QB_.Limit,Venues);
                Poco::JSON::Array   Arr;
                for(const auto &i:Venues) {
                    Poco::JSON::Object  O;
                    i.to_json(O);
                    Arr.add(O);
                }
                Poco::JSON::Object  Answer;
                Answer.set("venues",Arr);
                ReturnObject(Request, Answer, Response);
                return;
            }
        } catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response);
    }
}