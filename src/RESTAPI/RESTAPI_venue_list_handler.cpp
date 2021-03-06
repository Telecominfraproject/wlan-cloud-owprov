//
// Created by stephane bourque on 2021-08-23.
//

#include "RESTAPI_venue_list_handler.h"
#include "StorageService.h"
#include "RESTAPI/RESTAPI_db_helpers.h"

namespace OpenWifi{
    void RESTAPI_venue_list_handler::DoGet() {
        return ListHandler<VenueDB>("venues", DB_, *this);
    }
}