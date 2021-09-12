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
#include "RESTAPI_errors.h"

namespace OpenWifi{
    void RESTAPI_contact_handler::DoGet() {
        std::string UUID = GetBinding(RESTAPI::Protocol::ID,"");
        if(UUID.empty()) {
            BadRequest(RESTAPI::Errors::MissingUUID);
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
    }

    void RESTAPI_contact_handler::DoDelete() {
        std::string UUID = GetBinding("uuid","");
        if(UUID.empty()) {
            BadRequest(RESTAPI::Errors::MissingUUID);
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
            BadRequest(RESTAPI::Errors::StillInUse);
            return;
        }
    }

    void RESTAPI_contact_handler::DoPost() {
        std::string UUID = GetBinding(RESTAPI::Protocol::ID,"");
        if(UUID.empty()) {
            BadRequest(RESTAPI::Errors::MissingUUID);
            return;
        }

        auto Obj = ParseStream();
        ProvObjects::Contact C;
        if (!C.from_json(Obj)) {
            BadRequest(RESTAPI::Errors::InvalidJSONDocument);
            return;
        }

        C.info.id = Daemon()->CreateUUID();
        C.info.created = C.info.modified = std::time(nullptr);

        std::string f{RESTAPI::Protocol::ID};

        if(Storage()->ContactDB().CreateRecord(C)) {
            Poco::JSON::Object Answer;
            C.to_json(Answer);
            ReturnObject(Answer);
        } else {
            BadRequest(RESTAPI::Errors::RecordNotCreated);
        }
    }

    void RESTAPI_contact_handler::DoPut() {

    }
}