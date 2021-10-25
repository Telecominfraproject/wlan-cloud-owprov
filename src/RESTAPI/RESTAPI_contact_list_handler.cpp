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
            return ReturnRecordList<decltype(StorageService()->ContactDB()),
            ProvObjects::Contact>("contacts",StorageService()->ContactDB(),*this );
        } else if(QB_.CountOnly) {
            Poco::JSON::Object  Answer;
            auto C = StorageService()->ContactDB().Count();
            return ReturnCountOnly(C);
        } else {
            ProvObjects::ContactVec Contacts;
            StorageService()->ContactDB().GetRecords(QB_.Offset,QB_.Limit,Contacts);
            return MakeJSONObjectArray("contacts", Contacts, *this);
        }
    }
}