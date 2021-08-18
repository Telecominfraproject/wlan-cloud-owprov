//
// Created by stephane bourque on 2021-08-18.
//

#include "RESTAPI_entity_list_handler.h"
#include "Utils.h"
#include "StorageService.h"

namespace OpenWifi{
    void RESTAPI_entity_list_handler::handleRequest(Poco::Net::HTTPServerRequest &Request,
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

    void RESTAPI_entity_list_handler::DoGet(Poco::Net::HTTPServerRequest &Request,
                                               Poco::Net::HTTPServerResponse &Response) {
        try {
            if(!QB_.Select.empty()) {
                auto EntityUIDs = uCentral::Utils::Split(QB_.Select);
                Poco::JSON::Array   Arr;
                for(const auto &i:EntityUIDs) {
                    std::string f{"id"};
                    ProvObjects::Entity E;
                    if(Storage()->EntityDB().GetRecord(f,i,E)) {
                        Poco::JSON::Object  O;
                        E.to_json(O);
                        Arr.add(O);
                    } else {
                        BadRequest(Request, Response, "Unknown UUID:" + i);
                        return;
                    }
                }
                Poco::JSON::Object  Answer;
                Answer.set("entities",Arr);
                ReturnObject(Request, Answer, Response);
                return;
            } else {
                std::vector<ProvObjects::Entity> Entities;
                Storage()->EntityDB().GetRecords(QB_.Offset,QB_.Limit,Entities);
                Poco::JSON::Array   Arr;
                for(const auto &i:Entities) {
                    Poco::JSON::Object  O;
                    i.to_json(O);
                    Arr.add(O);
                }
                Poco::JSON::Object  Answer;
                Answer.set("entities",Arr);
                ReturnObject(Request, Answer, Response);
                return;
            }
        } catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response);
    }
}