//
// Created by stephane bourque on 2021-08-23.
//

#include "RESTAPI_contact_list_handler.h"
#include "Utils.h"
#include "RESTAPI_ProvObjects.h"
#include "StorageService.h"

namespace OpenWifi{
    void RESTAPI_contact_list_handler::handleRequest(Poco::Net::HTTPServerRequest &Request,
                                                       Poco::Net::HTTPServerResponse &Response) {
        if (!ContinueProcessing(Request, Response))
            return;

        if (!IsAuthorized(Request, Response))
            return;

        ParseParameters(Request);
        if(Request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET)
            DoGet(Request, Response);
        else
            BadRequest(Request, Response, "Unknown HTTP Method");
    }

    void RESTAPI_contact_list_handler::DoGet(Poco::Net::HTTPServerRequest &Request,
                                               Poco::Net::HTTPServerResponse &Response) {
        try {
            std::string UUID;
            std::string Arg;

            if(!QB_.Select.empty()) {
                auto DevUUIDS = Utils::Split(QB_.Select);
                ProvObjects::ContactVec Contacts;
                for(const auto &i:DevUUIDS) {
                    ProvObjects::Contact E;
                    if(Storage()->ContactDB().GetRecord("id",i,E)) {
                        Contacts.push_back(E);
                    } else {
                        BadRequest(Request, Response, "Unknown UUID:" + i);
                        return;
                    }
                }
                ReturnObject(Request, "contacts", Contacts, Response);
                return;
            } else if(HasParameter("countOnly",Arg) && Arg=="true") {
                Poco::JSON::Object  Answer;
                auto C = Storage()->ContactDB().Count();
                ReturnCountOnly(Request,C,Response);
                return;
            } else {
                ProvObjects::ContactVec Contacts;
                Storage()->ContactDB().GetRecords(QB_.Offset,QB_.Limit,Contacts);
                ReturnObject(Request, "contacts", Contacts, Response);
                return;
            }
        } catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response);
    }
}