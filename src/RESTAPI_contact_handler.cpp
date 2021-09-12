//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "RESTAPI_contact_handler.h"
#include "RESTAPI_protocol.h"
#include "RESTAPI_ProvObjects.h"
#include "StorageService.h"
#include "Poco/JSON/Parser.h"
#include "Daemon.h"

namespace OpenWifi{
    void RESTAPI_contact_handler::DoGet() {
        try {
            std::string UUID = GetBinding(RESTAPI::Protocol::ID,"");
            if(UUID.empty()) {
                BadRequest("Missing UUID.");
                return;
            }

            ProvObjects::Contact   C;
            if(Storage()->ContactDB().GetRecord(RESTAPI::Protocol::ID,UUID,C)) {
                Poco::JSON::Object  Answer;
                C.to_json(Answer);
                ReturnObject(Answer);
                return;
            } else {
                NotFound();
                return;
            }
        }  catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest("Internal error. Try again.");
    }

    void RESTAPI_contact_handler::DoDelete() {
        try {
            std::string UUID = GetBinding("uuid","");
            if(UUID.empty()) {
                BadRequest("Missing UUID");
                return;
            }

            ProvObjects::Contact    ExistingContact;
            if(!Storage()->ContactDB().GetRecord("id",UUID,ExistingContact)) {
                NotFound();
                return;
            }

            bool Force=false;
            std::string Arg;
            if(HasParameter("force",Arg) && Arg=="true")
                Force=true;

            if(!Force && !ExistingContact.inUse.empty()) {
                BadRequest("Some entities still reference this entry. Delete them or use force=true");
                return;
            }

        } catch( const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest("An error occurred and the contact was not added.");
    }

    void RESTAPI_contact_handler::DoPost() {
        try {
            std::string UUID = GetBinding(RESTAPI::Protocol::ID,"");
            if(UUID.empty()) {
                BadRequest("Missing UUID.");
                return;
            }

            auto Obj = ParseStream();
            ProvObjects::Contact C;
            if (!C.from_json(Obj)) {
                BadRequest("Cannot parse incoming POST.");
                return;
            }

            C.info.id = Daemon()->CreateUUID();
            C.info.created = C.info.modified = std::time(nullptr);

            std::string f{RESTAPI::Protocol::ID};

            if(Storage()->ContactDB().CreateRecord(C)) {
                Poco::JSON::Object Answer;
                C.to_json(Answer);
                ReturnObject(Answer);
                return;
            }
        }  catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest("An error occurred and the contact was not added.");
    }

    void RESTAPI_contact_handler::DoPut() {

    }
}