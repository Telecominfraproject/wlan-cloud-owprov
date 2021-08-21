//
// Created by stephane bourque on 2021-08-17.
//

#include "RESTAPI_inventory_list_handler.h"
#include "StorageService.h"
#include "Utils.h"

namespace OpenWifi{
    void RESTAPI_inventory_list_handler::handleRequest(Poco::Net::HTTPServerRequest &Request,
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

    void RESTAPI_inventory_list_handler::DoGet(Poco::Net::HTTPServerRequest &Request,
                                        Poco::Net::HTTPServerResponse &Response) {

        try {
            if(!QB_.Select.empty()) {
                auto DevUIIDS = Utils::Split(QB_.Select);
                Poco::JSON::Array   Arr;
                for(const auto &i:DevUIIDS) {
                    ProvObjects::InventoryTag E;
                    if(Storage()->InventoryDB().GetRecord("id",i,E)) {
                        Poco::JSON::Object  O;
                        E.to_json(O);
                        Arr.add(O);
                    } else {
                        BadRequest(Request, Response, "Unknown UUID:" + i);
                        return;
                    }
                }
                Poco::JSON::Object  Answer;
                Answer.set("inventoryTags",Arr);
                ReturnObject(Request, Answer, Response);
                return;
            } else {
                std::vector<ProvObjects::InventoryTag> Tags;
                Storage()->InventoryDB().GetRecords(QB_.Offset,QB_.Limit,Tags);
                Poco::JSON::Array   Arr;
                for(const auto &i:Tags) {
                    Poco::JSON::Object  O;
                    i.to_json(O);
                    Arr.add(O);
                }
                Poco::JSON::Object  Answer;
                Answer.set("inventoryTags",Arr);
                ReturnObject(Request, Answer, Response);
                return;
            }
        } catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response);
    }
}