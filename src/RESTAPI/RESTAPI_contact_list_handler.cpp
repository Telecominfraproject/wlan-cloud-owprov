//
// Created by stephane bourque on 2021-08-23.
//

#include "RESTAPI_contact_list_handler.h"
#include "framework/Utils.h"
#include "RESTAPI_ProvObjects.h"
#include "StorageService.h"
#include "framework/RESTAPI_errors.h"
#include "RESTAPI_db_helpers.h"

namespace OpenWifi{
    void RESTAPI_contact_list_handler::DoGet() {
        if(!QB_.Select.empty()) {
            auto DevUUIDS = Utils::Split(QB_.Select);
            Poco::JSON::Array   ObjArr;
            for(const auto &i:DevUUIDS) {
                ProvObjects::Contact E;
                Poco::JSON::Object  Obj;
                if(Storage()->ContactDB().GetRecord("id",i,E)) {
                    E.to_json(Obj);
                    if(QB_.AdditionalInfo)
                        AddContactExtendedInfo(E, Obj);
                    ObjArr.add(Obj);
                } else {
                    return BadRequest(RESTAPI::Errors::ContactMustExist + " ("+i+")");
                }
            }
            Poco::JSON::Object  Answer;
            Answer.set("contacts", ObjArr);
            return ReturnObject(Answer);
        } else if(QB_.CountOnly) {
            Poco::JSON::Object  Answer;
            auto C = Storage()->ContactDB().Count();
            ReturnCountOnly(C);
            return;
        } else {
            ProvObjects::ContactVec Contacts;
            Storage()->ContactDB().GetRecords(QB_.Offset,QB_.Limit,Contacts);
            Poco::JSON::Array   ObjArray;
            for(const auto &i:Contacts) {
                Poco::JSON::Object  Obj;
                i.to_json(Obj);
                if(QB_.AdditionalInfo)
                    AddContactExtendedInfo(i, Obj);
                ObjArray.add(Obj);
            }
            Poco::JSON::Object  Answer;
            Answer.set("contacts",ObjArray);
            return ReturnObject(Answer);
        }
    }
}