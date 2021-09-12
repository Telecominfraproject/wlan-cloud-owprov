//
// Created by stephane bourque on 2021-08-23.
//

#include "RESTAPI_contact_list_handler.h"
#include "Utils.h"
#include "RESTAPI_ProvObjects.h"
#include "StorageService.h"

namespace OpenWifi{
    void RESTAPI_contact_list_handler::DoGet() {
        try {
            if(!QB_.Select.empty()) {
                auto DevUUIDS = Utils::Split(QB_.Select);
                ProvObjects::ContactVec Contacts;
                for(const auto &i:DevUUIDS) {
                    ProvObjects::Contact E;
                    if(Storage()->ContactDB().GetRecord("id",i,E)) {
                        Contacts.push_back(E);
                    } else {
                        BadRequest("Unknown UUID:" + i);
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
        } catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest("Internal error. Please try again.");
    }

    void RESTAPI_contact_list_handler::DoPost() {};
    void RESTAPI_contact_list_handler::DoPut() {};
    void RESTAPI_contact_list_handler::DoDelete() {};

}