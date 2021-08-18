//
// Created by stephane bourque on 2021-08-16.
//

#include "RESTAPI_entity_handler.h"
#include "RESTAPI_ProvObjects.h"
#include "StorageService.h"

#include "Poco/JSON/Parser.h"
#include "Daemon.h"

namespace OpenWifi{

    void RESTAPI_entity_handler::handleRequest(Poco::Net::HTTPServerRequest &Request,
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

    void RESTAPI_entity_handler::DoGet(Poco::Net::HTTPServerRequest &Request,
                                                 Poco::Net::HTTPServerResponse &Response) {
        try {
            std::string UUID = GetBinding("uuid", "");

            if(UUID.empty()) {
                BadRequest(Request, Response, "Missing UUID");
                return;
            }

            ProvObjects::Entity E;
            if(Storage()->EntityDB().GetRecord("id",UUID,E)) {
                Poco::JSON::Object Answer;
                E.to_json(Answer);
                ReturnObject(Request,Answer,Response);
                return;
            }
            NotFound(Request,Response);
            return;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response);
    }

    void RESTAPI_entity_handler::DoDelete(Poco::Net::HTTPServerRequest &Request,
                                                    Poco::Net::HTTPServerResponse &Response) {
        try {
            std::string UUID = GetBinding("uuid", "");

            if(UUID.empty()) {
                BadRequest(Request, Response, "Missing UUID");
                return;
            }

            if(UUID == EntityDB::RootUUID()) {
                BadRequest(Request, Response, "Root Entity cannot be removed, only modified");
                return;
            }

            ProvObjects::Entity E;
            if(Storage()->EntityDB().DeleteRecord("id",UUID)) {
                OK(Request, Response);
                return;
            }
            NotFound(Request,Response);
            return;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response);
    }

    void RESTAPI_entity_handler::DoPost(Poco::Net::HTTPServerRequest &Request,
                                                  Poco::Net::HTTPServerResponse &Response) {
        try {
            std::cout << __LINE__ << std::endl;
            std::string UUID = GetBinding("uuid", "");

            std::cout << __LINE__ << std::endl;
            if(UUID.empty()) {
                BadRequest(Request, Response, "Missing UUID");
                std::cout << __LINE__ << std::endl;
                return;
            }

            std::cout << __LINE__ << std::endl;
            if(!Storage()->EntityDB().RootExists() && UUID != EntityDB::RootUUID()) {
                BadRequest(Request, Response, "Root entity must be created first.");
                std::cout << __LINE__ << std::endl;
                return;
            }

            std::cout << __LINE__ << std::endl;
            Poco::JSON::Parser IncomingParser;
            std::cout << __LINE__ << std::endl;
            Poco::JSON::Object::Ptr Obj = IncomingParser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
            std::cout << __LINE__ << std::endl;
            ProvObjects::Entity E;
            std::cout << __LINE__ << std::endl;
            if (!E.from_json(Obj)) {
                std::cout << __LINE__ << std::endl;
                BadRequest(Request, Response);
                return;
            }

            //  When creating an entity, it cannot have any relations other that parent, notes, name, description. Everything else
            //  must be conveyed through PUT.
            E.info.id = (UUID==EntityDB::RootUUID()) ? UUID : uCentral::Daemon()->CreateUUID() ;
            if(UUID==EntityDB::RootUUID())
                E.parent="";
            E.venues.clear();
            E.children.clear();
            E.managers.clear();
            E.contacts.clear();
            E.locations.clear();
            std::cout << __LINE__ << std::endl;


            if(Storage()->EntityDB().CreateRecord(E)) {
                std::cout << __LINE__ << std::endl;
                if(UUID==EntityDB::RootUUID())
                    Storage()->EntityDB().CheckForRoot();
                std::cout << __LINE__ << std::endl;
                OK(Request, Response);
                return;
            }
            std::cout << __LINE__ << std::endl;
            NotFound(Request,Response);
            return;
        } catch (const Poco::Exception &E) {
            std::cout << __LINE__ << std::endl;
            Logger_.log(E);
        }
        std::cout << __LINE__ << std::endl;
        BadRequest(Request, Response);
    }

    void RESTAPI_entity_handler::DoPut(Poco::Net::HTTPServerRequest &Request,
                                                 Poco::Net::HTTPServerResponse &Response) {}
}