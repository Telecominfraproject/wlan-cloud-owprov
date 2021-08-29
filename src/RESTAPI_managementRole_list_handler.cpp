//
// Created by stephane bourque on 2021-08-26.
//

#include "RESTAPI_managementRole_list_handler.h"

#include "Utils.h"
#include "RESTAPI_ProvObjects.h"
#include "StorageService.h"

namespace OpenWifi{
    void RESTAPI_managementRole_list_handler::handleRequest(Poco::Net::HTTPServerRequest &Request,
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

    void RESTAPI_managementRole_list_handler::DoGet(Poco::Net::HTTPServerRequest &Request,
                                                      Poco::Net::HTTPServerResponse &Response) {
        try {
            std::string UUID;
            std::string Arg;

            if(!QB_.Select.empty()) {
                auto DevUUIDS = Utils::Split(QB_.Select);
                ProvObjects::ManagementRoleVec Roles;
                for(const auto &i:DevUUIDS) {
                    ProvObjects::ManagementRole E;
                    if(Storage()->RolesDB().GetRecord("id",i,E)) {
                        Roles.push_back(E);
                    } else {
                        BadRequest(Request, Response, "Unknown UUID:" + i);
                        return;
                    }
                }
                ReturnObject(Request, "roles", Roles, Response);
                return;
            } else if(QB_.CountOnly) {
                Poco::JSON::Object  Answer;
                auto C = Storage()->RolesDB().Count();
                ReturnCountOnly(Request,C,Response);
                return;
            } else {
                ProvObjects::ManagementRoleVec Roles;
                Storage()->RolesDB().GetRecords(QB_.Offset,QB_.Limit,Roles);
                ReturnObject(Request, "roles", Roles, Response);
                return;
            }
        } catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response);
    }
}