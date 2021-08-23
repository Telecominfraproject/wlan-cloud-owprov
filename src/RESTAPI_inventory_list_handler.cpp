//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
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
            std::string UUID;
            if(!QB_.Select.empty()) {
                auto DevUUIDS = Utils::Split(QB_.Select);
                Poco::JSON::Array   Arr;
                for(const auto &i:DevUUIDS) {
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
                Answer.set("tags",Arr);
                ReturnObject(Request, Answer, Response);
                return;
            } else if(HasParameter("entity",UUID)) {
                std::string Arg;
                if(HasParameter("countOnly",Arg) && Arg=="true") {
                    Poco::JSON::Object  Answer;
                    auto C = Storage()->InventoryDB().Count(Storage()->InventoryDB().MakeWhere("entity",ORM::EQUAL,UUID));
                    Answer.set("count", C);
                    ReturnObject(Request, Answer, Response);
                    return;
                }
                bool SerialOnly=false;
                if(HasParameter("serialOnly",Arg) && Arg=="true")
                    SerialOnly=true;
                InventoryTagVec Tags;
                Storage()->InventoryDB().GetRecords(QB_.Offset, QB_.Limit, Tags, Storage()->InventoryDB().MakeWhere("entity",ORM::EQUAL,UUID));
                Poco::JSON::Array   Array;
                for(const auto &i:Tags) {
                    if(SerialOnly) {
                        Array.add(i.serialNumber);
                    } else {
                        Poco::JSON::Object  O;
                        i.to_json(O);
                        Array.add(O);
                    }
                }
                Poco::JSON::Object  Answer;
                if(SerialOnly)
                    Answer.set("serialNumbers", Array);
                else
                    Answer.set("tags", Array);
                ReturnObject(Request, Answer, Response);
                return;
            } else if(HasParameter("venue",UUID)) {
                std::string Arg;
                if(HasParameter("countOnly",Arg) && Arg=="true") {
                    Poco::JSON::Object  Answer;
                    auto C = Storage()->InventoryDB().Count(Storage()->InventoryDB().MakeWhere("venue",ORM::EQUAL,UUID));
                    Answer.set("count", C);
                    ReturnObject(Request, Answer, Response);
                    return;
                }
                bool SerialOnly=false;
                if(HasParameter("serialOnly",Arg) && Arg=="true")
                    SerialOnly=true;
                InventoryTagVec Tags;
                Storage()->InventoryDB().GetRecords(QB_.Offset, QB_.Limit, Tags, Storage()->InventoryDB().MakeWhere("venue",ORM::EQUAL,UUID));
                Poco::JSON::Array   Array;
                for(const auto &i:Tags) {
                    if(SerialOnly) {
                        Array.add(i.serialNumber);
                    } else {
                        Poco::JSON::Object  O;
                        i.to_json(O);
                        Array.add(O);
                    }
                }
                Poco::JSON::Object  Answer;
                if(SerialOnly)
                    Answer.set("serialNumbers", Array);
                else
                    Answer.set("tags", Array);
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
                Answer.set("tags",Arr);
                ReturnObject(Request, Answer, Response);
                return;
            }
        } catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response);
    }
}