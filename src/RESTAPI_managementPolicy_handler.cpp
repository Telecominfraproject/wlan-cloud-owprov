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
#include "RESTAPI_errors.h"

namespace OpenWifi{

    void RESTAPI_managementPolicy_handler::DoGet() {
        std::string UUID = GetBinding("uuid","");
        ProvObjects::ManagementPolicy   Existing;
        if(UUID.empty() || !DB_.GetRecord("id", UUID, Existing)) {
            return NotFound();
        }

        std::string Arg;
        if(HasParameter("expandInUse",Arg) && Arg=="true") {
            Storage::ExpandedListMap    M;
            std::vector<std::string>    Errors;
            Poco::JSON::Object  Inner;
            if(Storage()->ExpandInUse(Existing.inUse,M,Errors)) {
                for(const auto &[type,list]:M) {
                    Poco::JSON::Array   ObjList;
                    for(const auto &i:list.entries) {
                        Poco::JSON::Object  O;
                        i.to_json(O);
                        ObjList.add(O);
                    }
                    Inner.set(type,ObjList);
                }
            }
            Poco::JSON::Object  Answer;
            Answer.set("entries", Inner);
            return ReturnObject(Answer);
        }

        Poco::JSON::Object  Answer;
        Existing.to_json(Answer);
        ReturnObject(Answer);
    }

    void RESTAPI_managementPolicy_handler::DoDelete() {
        std::string UUID = GetBinding("uuid","");
        ProvObjects::ManagementPolicy   Existing;
        if(UUID.empty() || !DB_.GetRecord("id", UUID, Existing)) {
            return NotFound();
        }

        if(!Existing.inUse.empty()) {
            return BadRequest(RESTAPI::Errors::StillInUse);
        }

        if(Storage()->PolicyDB().DeleteRecord("id", UUID)) {
            return OK();
        }

        InternalError(RESTAPI::Errors::CouldNotBeDeleted);
    }

    void RESTAPI_managementPolicy_handler::DoPost() {
        std::string UUID = GetBinding("uuid","");
        if(UUID.empty()) {
            return BadRequest(RESTAPI::Errors::MissingUUID);
        }

        ProvObjects::ManagementPolicy   NewPolicy;
        auto NewObj = ParseStream();
        if(!NewPolicy.from_json(NewObj)) {
            return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
        }

        if(NewPolicy.info.name.empty()) {
            return BadRequest(RESTAPI::Errors::NameMustBeSet);
        }

        NewPolicy.inUse.clear();
        NewPolicy.info.id = Daemon()->CreateUUID();
        NewPolicy.info.created = NewPolicy.info.modified = std::time(nullptr);
        if(DB_.CreateRecord(NewPolicy)) {
            ProvObjects::ManagementPolicy   Policy;
            DB_.GetRecord("id",NewPolicy.info.id,Policy);
            Poco::JSON::Object  Answer;
            Policy.to_json(Answer);
            return ReturnObject(Answer);
        }
        InternalError(RESTAPI::Errors::RecordNotCreated);
    }

    void RESTAPI_managementPolicy_handler::DoPut() {
        std::string UUID = GetBinding("uuid","");
        ProvObjects::ManagementPolicy   Existing;
        if(UUID.empty() || !DB_.GetRecord("id", UUID, Existing)) {
            return NotFound();
        }

        ProvObjects::ManagementPolicy   NewPolicy;
        auto NewObj = ParseStream();
        if(!NewPolicy.from_json(NewObj)) {
            return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
        }

        AssignIfPresent(NewObj, "name", Existing.info.name);
        AssignIfPresent(NewObj, "description", Existing.info.description);
        Existing.info.modified = std::time(nullptr);

        if(!NewPolicy.entries.empty())
            Existing.entries = NewPolicy.entries;

        if(DB_.UpdateRecord("id", Existing.info.id, Existing)) {
           ProvObjects::ManagementPolicy   P;
            DB_.GetRecord("id",Existing.info.id,P);
            Poco::JSON::Object  Answer;
            P.to_json(Answer);
            return ReturnObject(Answer);
        }
        InternalError(RESTAPI::Errors::RecordNotUpdated);
    }
}