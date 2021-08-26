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
    void RESTAPI_contact_handler::handleRequest(Poco::Net::HTTPServerRequest &Request,
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

    void RESTAPI_contact_handler::DoGet(Poco::Net::HTTPServerRequest &Request,
                                       Poco::Net::HTTPServerResponse &Response) {
        try {
            std::string UUID = GetBinding(RESTAPI::Protocol::ID,"");
            if(UUID.empty()) {
                BadRequest(Request, Response, "Missing UUID.");
                return;
            }

            ProvObjects::Contact   C;
            if(Storage()->ContactDB().GetRecord(RESTAPI::Protocol::ID,UUID,C)) {
                Poco::JSON::Object  Answer;
                C.to_json(Answer);
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

    void RESTAPI_contact_handler::DoDelete(Poco::Net::HTTPServerRequest &Request,
                                         Poco::Net::HTTPServerResponse &Response) {
        try {
            std::string UUID = GetBinding("uuid","");
            if(UUID.empty()) {
                BadRequest(Request, Response, "Missing UUID");
                return;
            }

            ProvObjects::Contact    ExistingContact;
            if(!Storage()->ContactDB().GetRecord("id",UUID,ExistingContact)) {
                NotFound(Request, Response);
                return;
            }

            bool Force=false;
            std::string Arg;
            if(HasParameter("force",Arg) && Arg=="true")
                Force=true;

            if(!Force && !ExistingContact.inUse.empty()) {
                BadRequest(Request, Response, "Some entities still reference this entry. Delete them or use force=true");
                return;
            }

        } catch( const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response, "An error occurred and the contact was not added.");
    }

    void RESTAPI_contact_handler::DoPost(Poco::Net::HTTPServerRequest &Request,
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

    void RESTAPI_contact_handler::DoPut(Poco::Net::HTTPServerRequest &Request,
                                    Poco::Net::HTTPServerResponse &Response) {

    }
}