//
// Created by stephane bourque on 2021-08-23.
//

#include "RESTAPI_venue_list_handler.h"
#include "Utils.h"
#include "StorageService.h"
#include "RESTAPI_utils.h"

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
                ProvObjects::VenueVec  Venues;
                for(const auto &i:UUIDs) {
                    ProvObjects::Venue E;
                    if(Storage()->VenueDB().GetRecord("id",i,E)) {
                        Venues.push_back(E);
                    } else {
                        BadRequest(Request, Response, "Unknown UUID:" + i);
                        return;
                    }
                }
                ReturnObject(Request, "venues", Venues, Response);
                return;
            } else if(HasParameter("entity",Arg)) {
                ProvObjects::VenueVec Venues;
                Storage()->VenueDB().GetRecords(QB_.Offset,QB_.Limit,Venues,Storage()->VenueDB().MakeWhere("entity", ORM::EQUAL, Arg));
                if(QB_.CountOnly) {
                    ReturnCountOnly(Request,Venues.size(),Response);
                } else {
                    ReturnObject(Request, "venues", Venues, Response);
                }
                return;
            } else if(HasParameter("venue",Arg)) {
                ProvObjects::VenueVec Venues;
                Storage()->VenueDB().GetRecords(QB_.Offset,QB_.Limit,Venues,Storage()->VenueDB().MakeWhere("venue", ORM::EQUAL, Arg));
                if(QB_.CountOnly) {
                    ReturnCountOnly(Request,Venues.size(),Response);
                } else {
                    ReturnObject(Request, "venues", Venues, Response);
                }
                return;
            } else if(QB_.CountOnly) {
                Poco::JSON::Object  Answer;
                auto C = Storage()->VenueDB().Count();
                ReturnCountOnly(Request, C,Response);
                return;
            } else {
                ProvObjects::VenueVec Venues;
                Storage()->VenueDB().GetRecords(QB_.Offset, QB_.Limit,Venues);
                ReturnObject(Request, "venues", Venues, Response);
                return;
            }
        } catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response);
    }
}