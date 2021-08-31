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
    void RESTAPI_managementRole_handler::handleRequest(Poco::Net::HTTPServerRequest &Request,
                                                Poco::Net::HTTPServerResponse &Response) {
        if (!ContinueProcessing(Request, Response))
            return;

        if (!IsAuthorized(Request, Response))
            return;

        ParseParameters(Request);
        if(Request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET)
            DoGet(Request, Response);
        else if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST)
            DoPost(Request, Response);
        else if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_DELETE)
            DoDelete(Request, Response);
        else if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_PUT)
            DoPut(Request, Response);
        else
            BadRequest(Request, Response, "Unknown HTTP Method");
    }

    void RESTAPI_managementRole_handler::DoGet(Poco::Net::HTTPServerRequest &Request,
                                        Poco::Net::HTTPServerResponse &Response) {
        try {
            std::string UUID = GetBinding(RESTAPI::Protocol::ID,"");
            if(UUID.empty()) {
                BadRequest(Request, Response, "Missing UUID.");
                return;
            }

            ProvObjects::ManagementRole   M;
            if(Storage()->RolesDB().GetRecord(RESTAPI::Protocol::ID,UUID,M)) {
                Poco::JSON::Object  Answer;
                M.to_json(Answer);
                ReturnObject(Request, Answer, Response);
                return;
            } else {
                NotFound(Request,Response);
                return;
            }
        }  catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response);
    }

    void RESTAPI_managementRole_handler::DoDelete(Poco::Net::HTTPServerRequest &Request,
                                           Poco::Net::HTTPServerResponse &Response) {
        try {
            std::string UUID = GetBinding("uuid","");
            if(UUID.empty()) {
                BadRequest(Request, Response, "Missing UUID");
                return;
            }

            ProvObjects::ManagementRole    ExistingManagementRole;
            if(!Storage()->RolesDB().GetRecord("id",UUID,ExistingManagementRole)) {
                NotFound(Request, Response);
                return;
            }

            bool Force=false;
            std::string Arg;
            if(HasParameter("force",Arg) && Arg=="true")
                Force=true;

            if(!Force && !ExistingManagementRole.inUse.empty()) {
                BadRequest(Request, Response, "Some entities still reference this entry. Delete them or use force=true");
                return;
            }

            Storage()->PolicyDB().DeleteInUse("id", ExistingManagementRole.managementPolicy, Storage()->RolesDB().Prefix(), ExistingManagementRole.info.id);
            if(Storage()->RolesDB().DeleteRecord("id", ExistingManagementRole.info.id)) {
                OK(Request,Response);
            }
        } catch( const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response, "An error occurred and the contact was not added.");
    }

    void RESTAPI_managementRole_handler::DoPost(Poco::Net::HTTPServerRequest &Request,
                                         Poco::Net::HTTPServerResponse &Response) {
        try {
            std::string UUID = GetBinding(RESTAPI::Protocol::ID,"");
            if(UUID.empty()) {
                BadRequest(Request, Response, "Missing UUID.");
                return;
            }

            Poco::JSON::Parser IncomingParser;
            Poco::JSON::Object::Ptr Obj = IncomingParser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
            ProvObjects::Contact C;
            if (!C.from_json(Obj)) {
                BadRequest(Request, Response, "Cannot parse incoming POST.");
                return;
            }

            C.info.id = Daemon()->CreateUUID();
            C.info.created = C.info.modified = std::time(nullptr);

            std::string f{RESTAPI::Protocol::ID};

            if(Storage()->ContactDB().CreateRecord(C)) {
                Poco::JSON::Object Answer;
                C.to_json(Answer);
                ReturnObject(Request, Answer, Response);
                return;
            }
        }  catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response, "An error occurred and the contact was not added.");
    }

    void RESTAPI_managementRole_handler::DoPut(Poco::Net::HTTPServerRequest &Request,
                                        Poco::Net::HTTPServerResponse &Response) {
        try {
            std::string UUID = GetBinding(RESTAPI::Protocol::ID,"");
            if(UUID.empty()) {
                BadRequest(Request, Response, "Missing UUID.");
                return;
            }

            ProvObjects::ManagementRole Existing;
            if(!Storage()->RolesDB().GetRecord("id",UUID,Existing)) {
                NotFound(Request,Response);
                return;
            }

            Poco::JSON::Parser IncomingParser;
            Poco::JSON::Object::Ptr RawObject = IncomingParser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
            if(RawObject->has("notes")) {
                SecurityObjects::append_from_json(RawObject, UserInfo_.userinfo, Existing.info.notes);
            }

            AssignIfPresent(RawObject, "name", Existing.info.name);
            AssignIfPresent(RawObject, "description", Existing.info.description);

            std::string NewPolicy,OldPolicy = Existing.managementPolicy;
            AssignIfPresent(RawObject, "managementPolicy", NewPolicy);
            if(!NewPolicy.empty() && !Storage()->PolicyDB().Exists("id",NewPolicy)) {
                BadRequest(Request, Response, "Unknown Policy:" + NewPolicy);
                return;
            }

            std::string Error;
            if(!Storage()->Validate(Parameters_,Error)) {
                BadRequest(Request, Response, "Unknown users: " + Error);
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
                ReturnObject(Request, Answer, Response);
                return;
            }
        }  catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response, "An error occurred and the contact was not added.");
    }
}