//
// Created by stephane bourque on 2021-08-23.
//

#include "RESTAPI_contact_list_handler.h"
#include "Utils.h"
#include "RESTAPI_ProvObjects.h"
#include "StorageService.h"
#include "RESTAPI_errors.h"

namespace OpenWifi{
    void RESTAPI_contact_list_handler::DoGet() {
        if(!QB_.Select.empty()) {
            auto DevUUIDS = Utils::Split(QB_.Select);
            ProvObjects::ContactVec Contacts;
            for(const auto &i:DevUUIDS) {
                ProvObjects::Contact E;
                if(Storage()->ContactDB().GetRecord("id",i,E)) {
                    Contacts.push_back(E);
                } else {
                    BadRequest(RESTAPI::Errors::UnknownId + " ("+i+")");
                    return;
                }
            }
            ReturnObject("contacts", Contacts);
            return;
        } else if(QB_.CountOnly) {
            Poco::JSON::Object  Answer;
            auto C = Storage()->ContactDB().Count();
            ReturnCountOnly(C);
            return;
        } else {
            ProvObjects::ContactVec Contacts;
            Storage()->ContactDB().GetRecords(QB_.Offset,QB_.Limit,Contacts);
            ReturnObject("contacts", Contacts);
            return;
        }
    }
}