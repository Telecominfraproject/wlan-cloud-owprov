//
// Created by stephane bourque on 2021-08-23.
//

#include "RESTAPI_venue_list_handler.h"
#include "StorageService.h"
#include "RESTAPI/RESTAPI_db_helpers.h"

namespace OpenWifi{
    void RESTAPI_venue_list_handler::DoGet() {
        try {
            std::string Arg;
            if(!QB_.Select.empty()) {
                return ReturnRecordList<decltype(DB_),
                ProvObjects::Venue>("venues",DB_,*this );
            } else if(HasParameter("entity",Arg)) {
                ProvObjects::VenueVec Venues;
                DB_.GetRecords(QB_.Offset,QB_.Limit,Venues, DB_.OP("entity",ORM::EQ,Arg));
                if(QB_.CountOnly) {
                    return ReturnCountOnly(Venues.size());
                }
                return MakeJSONObjectArray("venues", Venues, *this);
            } else if(HasParameter("venue",Arg)) {
                VenueDB::RecordVec Venues;
                DB_.GetRecords(QB_.Offset,QB_.Limit,Venues,DB_.OP("venue",ORM::EQ,Arg));
                if(QB_.CountOnly) {
                    return ReturnCountOnly(Venues.size());
                }
                return MakeJSONObjectArray("venues", Venues, *this);
            } else if(QB_.CountOnly) {
                Poco::JSON::Object  Answer;
                auto C = DB_.Count();
                return ReturnCountOnly(C);
            } else {
                VenueDB::RecordVec Venues;
                DB_.GetRecords(QB_.Offset, QB_.Limit,Venues);
                return MakeJSONObjectArray("venues", Venues, *this);
            }
        } catch(const Poco::Exception &E) {
            Logger().log(E);
        }
        InternalError("Internal error.");
    }
}