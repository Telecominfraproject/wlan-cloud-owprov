//
// Created by stephane bourque on 2021-08-26.
//

#include "RESTAPI_managementRole_handler.h"

#include "RESTAPI_protocol.h"
#include "RESTAPI_ProvObjects.h"
#include "StorageService.h"
#include "Poco/JSON/Parser.h"
#include "Daemon.h"
#include "Poco/StringTokenizer.h"

namespace OpenWifi{

    void RESTAPI_managementRole_handler::DoGet() {
        try {
            std::string UUID = GetBinding(RESTAPI::Protocol::ID,"");
            if(UUID.empty()) {
                BadRequest("Missing UUID.");
                return;
            }

            ProvObjects::ManagementRole   M;
            if(Storage()->RolesDB().GetRecord(RESTAPI::Protocol::ID,UUID,M)) {
                Poco::JSON::Object  Answer;
                M.to_json(Answer);
                ReturnObject(Answer);
                return;
            } else {
                NotFound();
                return;
            }
        }  catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest("Internal error.");
    }

    void RESTAPI_managementRole_handler::DoDelete() {
        try {
            std::string UUID = GetBinding("uuid","");
            if(UUID.empty()) {
                BadRequest("Missing UUID");
                return;
            }

            ProvObjects::ManagementRole    ExistingManagementRole;
            if(!Storage()->RolesDB().GetRecord("id",UUID,ExistingManagementRole)) {
                NotFound();
                return;
            }

            bool Force=false;
            std::string Arg;
            if(HasParameter("force",Arg) && Arg=="true")
                Force=true;

            if(!Force && !ExistingManagementRole.inUse.empty()) {
                BadRequest("Some entities still reference this entry. Delete them or use force=true");
                return;
            }

            Storage()->PolicyDB().DeleteInUse("id", ExistingManagementRole.managementPolicy, Storage()->RolesDB().Prefix(), ExistingManagementRole.info.id);
            if(Storage()->RolesDB().DeleteRecord("id", ExistingManagementRole.info.id)) {
                OK();
            }
        } catch( const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest("An error occurred and the contact was not added.");
    }

    void RESTAPI_managementRole_handler::DoPost() {
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

    void RESTAPI_managementRole_handler::DoPut() {
        try {
            std::string UUID = GetBinding(RESTAPI::Protocol::ID,"");
            if(UUID.empty()) {
                BadRequest("Missing UUID.");
                return;
            }

            ProvObjects::ManagementRole Existing;
            if(!Storage()->RolesDB().GetRecord("id",UUID,Existing)) {
                NotFound();
                return;
            }

            auto RawObject = ParseStream();
            if(RawObject->has("notes")) {
                SecurityObjects::append_from_json(RawObject, UserInfo_.userinfo, Existing.info.notes);
            }

            AssignIfPresent(RawObject, "name", Existing.info.name);
            AssignIfPresent(RawObject, "description", Existing.info.description);

            std::string NewPolicy,OldPolicy = Existing.managementPolicy;
            AssignIfPresent(RawObject, "managementPolicy", NewPolicy);
            if(!NewPolicy.empty() && !Storage()->PolicyDB().Exists("id",NewPolicy)) {
                BadRequest("Unknown Policy:" + NewPolicy);
                return;
            }

            std::string Error;
            if(!Storage()->Validate(Parameters_,Error)) {
                BadRequest("Unknown users: " + Error);
                return;
            }

            if(!NewPolicy.empty())
                Existing.managementPolicy = NewPolicy;

            if(Storage()->RolesDB().UpdateRecord("id",UUID,Existing)) {
                if(!OldPolicy.empty())
                    Storage()->PolicyDB().DeleteInUse("id",OldPolicy,Storage()->RolesDB().Prefix(),UUID);
                for(const auto &i:Parameters_) {
                    if(i.first=="add") {
                        auto T = Poco::StringTokenizer(i.second,":");
                        if(T[0]==SecurityDBProxy()->Prefix()) {
                            Storage()->RolesDB().AddUser("id",UUID,T[1]);
                        }
                    } else if(i.second=="del") {
                        auto T = Poco::StringTokenizer(i.second,":");
                        if(T[0]==SecurityDBProxy()->Prefix()) {
                            Storage()->RolesDB().DelUser("id",UUID,T[1]);
                        }
                    }
                }
                Storage()->RolesDB().GetRecord("id", UUID, Existing);
                Poco::JSON::Object  Answer;
                Existing.to_json(Answer);
                ReturnObject(Answer);
                return;
            }
        }  catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest("An error occurred and the contact was not added.");
    }
}