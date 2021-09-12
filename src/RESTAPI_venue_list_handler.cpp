//
// Created by stephane bourque on 2021-08-23.
//

#include "RESTAPI_venue_list_handler.h"
#include "Utils.h"
#include "StorageService.h"
#include "RESTAPI_utils.h"

namespace OpenWifi{
    void RESTAPI_venue_list_handler::DoGet() {
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
                        BadRequest("Unknown UUID:" + i);
                        return;
                    }
                }
                ReturnObject("venues", Venues);
                return;
            } else if(HasParameter("entity",Arg)) {
                ProvObjects::VenueVec Venues;
                Storage()->VenueDB().GetRecords(QB_.Offset,QB_.Limit,Venues, Storage()->VenueDB().OP("entity",ORM::EQ,Arg));
                if(QB_.CountOnly) {
                    ReturnCountOnly(Venues.size());
                } else {
                    ReturnObject("venues", Venues);
                }
                return;
            } else if(HasParameter("venue",Arg)) {
                ProvObjects::VenueVec Venues;
                Storage()->VenueDB().GetRecords(QB_.Offset,QB_.Limit,Venues,Storage()->VenueDB().OP("venue",ORM::EQ,Arg));
                if(QB_.CountOnly) {
                    ReturnCountOnly(Venues.size());
                } else {
                    ReturnObject("venues", Venues);
                }
                return;
            } else if(QB_.CountOnly) {
                Poco::JSON::Object  Answer;
                auto C = Storage()->VenueDB().Count();
                ReturnCountOnly(C);
                return;
            } else {
                ProvObjects::VenueVec Venues;
                Storage()->VenueDB().GetRecords(QB_.Offset, QB_.Limit,Venues);
                ReturnObject("venues", Venues);
                return;
            }
        } catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest("Internal error.");
    }

    void RESTAPI_venue_list_handler::DoDelete() {}
    void RESTAPI_venue_list_handler::DoPut() {}
    void RESTAPI_venue_list_handler::DoPost() {}

}