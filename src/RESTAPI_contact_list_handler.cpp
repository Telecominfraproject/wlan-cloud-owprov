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
            std::string Arg;
            if(HasParameter("withExtendedInfo",Arg) && Arg=="true") {
                Poco::JSON::Array   ObjArray;
                for(const auto &i:Contacts) {
                    Poco::JSON::Object  Obj;
                    i.to_json(Obj);
                    Poco::JSON::Object  EI;
                    if(!i.entity.empty()) {
                        Poco::JSON::Object  EntObj;
                        ProvObjects::Entity Entity;
                        if(Storage()->EntityDB().GetRecord("id",i.entity,Entity)) {
                            EntObj.set( "name", Entity.info.name);
                            EntObj.set( "description", Entity.info.description);
                        }
                        EI.set("entity",EntObj);
                    }

                    if(!i.managementPolicy.empty()) {
                        Poco::JSON::Object  PolObj;
                        ProvObjects::ManagementPolicy Policy;
                        if(Storage()->PolicyDB().GetRecord("id",i.managementPolicy,Policy)) {
                            PolObj.set( "name", Policy.info.name);
                            PolObj.set( "description", Policy.info.description);
                        }
                        EI.set("managementPolicy",PolObj);
                    }
                    Obj.set("extendedInfo", EI);
                    ObjArray.add(Obj);
                }
                Poco::JSON::Object  Answer;
                Answer.set("contacts",ObjArray);
                return ReturnObject(Answer);
            }

            ReturnObject("contacts", Contacts);
            return;
        }
    }
}