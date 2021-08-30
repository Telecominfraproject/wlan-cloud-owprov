//
// Created by stephane bourque on 2021-08-26.
//

#include "RESTAPI_managementPolicy_list_handler.h"

#include "Utils.h"
#include "RESTAPI_ProvObjects.h"
#include "StorageService.h"

namespace OpenWifi{
    void RESTAPI_managementPolicy_list_handler::handleRequest(Poco::Net::HTTPServerRequest &Request,
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

    void RESTAPI_managementPolicy_list_handler::DoGet(Poco::Net::HTTPServerRequest &Request,
                                             Poco::Net::HTTPServerResponse &Response) {
        try {
            if(!QB_.Select.empty()) {
                auto DevUUIDS = Utils::Split(QB_.Select);
                ProvObjects::ManagementPolicyVec Policies;
                for(const auto &i:DevUUIDS) {
                    ProvObjects::ManagementPolicy E;
                    if(Storage()->PolicyDB().GetRecord("id",i,E)) {
                        Policies.push_back(E);
                    } else {
                        BadRequest(Request, Response, "Unknown UUID:" + i);
                        return;
                    }
                }
                ReturnObject(Request, "managementPolicies", Policies, Response);
                return;
            } else if(QB_.CountOnly) {
                Poco::JSON::Object  Answer;
                auto C = Storage()->ContactDB().Count();
                ReturnCountOnly(Request,C,Response);
                return;
            } else {
                ProvObjects::ManagementPolicyVec Policies;
                Storage()->PolicyDB().GetRecords(QB_.Offset,QB_.Limit,Policies);
                ReturnObject(Request, "managementPolicies", Policies, Response);
                return;
            }
        } catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response);
    }
}