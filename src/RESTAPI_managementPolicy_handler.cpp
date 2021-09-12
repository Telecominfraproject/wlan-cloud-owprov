//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//


#include "RESTAPI_managementPolicy_handler.h"
#include "RESTAPI_ProvObjects.h"
#include "StorageService.h"
#include "Poco/JSON/Parser.h"
#include "Daemon.h"

namespace OpenWifi{

    void RESTAPI_managementPolicy_handler::DoGet() {
        try {
            std::string UUID = GetBinding("uuid","");

            if(UUID.empty()) {
                BadRequest("Missing UUID.");
                return;
            }

            ProvObjects::ManagementPolicy   Existing;
            if(!Storage()->PolicyDB().GetRecord("id", UUID, Existing)) {
                NotFound();
                return;
            }

            Poco::JSON::Object  Answer;
            Existing.to_json(Answer);
            ReturnObject(Answer);
            return;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest("Internal error. Please try again.");
    }

    void RESTAPI_managementPolicy_handler::DoDelete() {
        try {
            std::string UUID = GetBinding("uuid","");

            if(UUID.empty()) {
                BadRequest("Missing UUID.");
                return;
            }

            ProvObjects::ManagementPolicy   Existing;
            if(!Storage()->PolicyDB().GetRecord("id", UUID, Existing)) {
                NotFound();
                return;
            }

            if(!Existing.inUse.empty()) {
                BadRequest("Cannot delete policy while still in use.");
                return;
            }

            if(Storage()->PolicyDB().DeleteRecord("id", UUID)) {
                OK();
                return;
            }
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest("Internal error. Please try again.");
    }

    void RESTAPI_managementPolicy_handler::DoPost() {
        try {

            std::string UUID = GetBinding("uuid","");
            if(UUID.empty()) {
                BadRequest("Missing UUID.");
                return;
            }

            ProvObjects::ManagementPolicy   NewPolicy;
            auto NewObj = ParseStream();
            if(!NewPolicy.from_json(NewObj)) {
                BadRequest("Ill formed JSON document.");
                return;
            }

            if(NewPolicy.info.name.empty()) {
                BadRequest("Name cannot be blank.");
                return;
            }

            NewPolicy.inUse.clear();
            NewPolicy.info.id = Daemon()->CreateUUID();
            NewPolicy.info.created = NewPolicy.info.modified = std::time(nullptr);
            if(Storage()->PolicyDB().CreateRecord(NewPolicy)) {
                ProvObjects::ManagementPolicy   Policy;
                Storage()->PolicyDB().GetRecord("id",NewPolicy.info.id,Policy);
                Poco::JSON::Object  Answer;
                Policy.to_json(Answer);
                ReturnObject(Answer);
                return;
            }
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest("Internal error. Please try again.");

    }

    void RESTAPI_managementPolicy_handler::DoPut() {

    }
}