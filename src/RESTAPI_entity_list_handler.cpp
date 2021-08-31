//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//


#include "RESTAPI_entity_list_handler.h"
#include "Utils.h"
#include "StorageService.h"
#include "RESTAPI_utils.h"

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
            std::string Arg;
            if(!QB_.Select.empty()) {
                auto EntityUIDs = Utils::Split(QB_.Select);
                ProvObjects::EntityVec Entities;
                for(const auto &i:EntityUIDs) {
                    ProvObjects::Entity E;
                    if(Storage()->EntityDB().GetRecord("id",i,E)) {
                        Entities.push_back(E);
                    } else {
                        BadRequest(Request, Response, "Unknown UUID:" + i);
                        return;
                    }
                }
                ReturnObject(Request, "entities", Entities, Response);
                return;
            } else if(QB_.CountOnly) {
                auto C = Storage()->EntityDB().Count();
                ReturnCountOnly(Request, C, Response);
                return;
            } if (HasParameter("getTree",Arg) && Arg=="true") {
                Poco::JSON::Object  FullTree;
                Storage()->EntityDB().BuildTree(FullTree);
                ReturnObject(Request,FullTree,Response);
                return;
            } else {
                ProvObjects::EntityVec Entities;
                Storage()->EntityDB().GetRecords(QB_.Offset, QB_.Limit,Entities);
                ReturnObject(Request,"entities",Entities,Response);
                return;
            }
        } catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response);
    }
}