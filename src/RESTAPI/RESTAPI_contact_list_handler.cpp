//
// Created by stephane bourque on 2021-08-23.
//

#include "RESTAPI_contact_list_handler.h"

#include "RESTObjects/RESTAPI_ProvObjects.h"
#include "StorageService.h"
#include "RESTAPI_db_helpers.h"

namespace OpenWifi{
    void RESTAPI_contact_list_handler::DoGet() {
        if(!QB_.Select.empty()) {
            return ReturnRecordList<decltype(DB_),
            ProvObjects::Contact>("contacts",DB_,*this );
        } else if(QB_.CountOnly) {
            Poco::JSON::Object  Answer;
            auto C = DB_.Count();
            return ReturnCountOnly(C);
        } else {
            ContactDB::RecordVec Contacts;
            DB_.GetRecords(QB_.Offset,QB_.Limit,Contacts);
            return MakeJSONObjectArray("contacts", Contacts, *this);
        }
    }
}