//
// Created by stephane bourque on 2021-08-23.
//

#include "RESTAPI_venue_list_handler.h"
#include "framework/Utils.h"
#include "StorageService.h"
#include "framework/RESTAPI_utils.h"
#include "framework/RESTAPI_errors.h"
#include "RESTAPI/RESTAPI_db_helpers.h"

namespace OpenWifi{
    void RESTAPI_venue_list_handler::DoGet() {
        try {
            std::string Arg;
            if(!QB_.Select.empty()) {
                auto UUIDs = Utils::Split(QB_.Select);
                Poco::JSON::Array   ObjArr;
                for(const auto &i:UUIDs) {
                    ProvObjects::Venue E;
                    if(Storage()->VenueDB().GetRecord("id",i,E)) {
                        Poco::JSON::Object  Obj;
                        E.to_json(Obj);
                        if(QB_.AdditionalInfo)
                            AddVenueExtendedInfo(E,Obj);
                        ObjArr.add(Obj);
                    } else {
                        return BadRequest("Unknown UUID:" + i);
                    }
                }
                Poco::JSON::Object  Answer;
                Answer.set("venues", ObjArr);
                return ReturnObject(Answer);
            } else if(HasParameter("entity",Arg)) {
                ProvObjects::VenueVec Venues;
                Storage()->VenueDB().GetRecords(QB_.Offset,QB_.Limit,Venues, Storage()->VenueDB().OP("entity",ORM::EQ,Arg));
                if(QB_.CountOnly) {
                    return ReturnCountOnly(Venues.size());
                } else {
                    return ReturnObject("venues", Venues);
                }
            } else if(HasParameter("venue",Arg)) {
                ProvObjects::VenueVec Venues;
                Storage()->VenueDB().GetRecords(QB_.Offset,QB_.Limit,Venues,Storage()->VenueDB().OP("venue",ORM::EQ,Arg));
                if(QB_.CountOnly) {
                    return ReturnCountOnly(Venues.size());
                } else {
                    return ReturnObject("venues", Venues);
                }
            } else if(QB_.CountOnly) {
                Poco::JSON::Object  Answer;
                auto C = Storage()->VenueDB().Count();
                return ReturnCountOnly(C);
            } else {
                ProvObjects::VenueVec Venues;
                Storage()->VenueDB().GetRecords(QB_.Offset, QB_.Limit,Venues);

                Poco::JSON::Array   ObjArr;
                for(const auto &i:Venues) {
                    Poco::JSON::Object  Obj;
                    i.to_json(Obj);
                    if(QB_.AdditionalInfo)
                        AddVenueExtendedInfo(i, Obj);
                    ObjArr.add(Obj);
                }
                Poco::JSON::Object  Answer;
                Answer.set("venues", ObjArr);
                return ReturnObject(Answer);
            }
        } catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        InternalError("Internal error.");
    }
}