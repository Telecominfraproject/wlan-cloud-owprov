//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "RESTAPI_configurations_handler.h"
#include "RESTAPI_ProvObjects.h"
#include "StorageService.h"
#include "Daemon.h"

namespace OpenWifi{
    void RESTAPI_configurations_handler::handleRequest(Poco::Net::HTTPServerRequest &Request,
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

    void RESTAPI_configurations_handler::DoGet(Poco::Net::HTTPServerRequest &Request,
                                                 Poco::Net::HTTPServerResponse &Response) {
        try {
            auto UUID = GetBinding("uuid","");
            if(UUID.empty()) {
                BadRequest(Request, Response, "Missing UUID.");
                return;
            }

            ProvObjects::DeviceConfiguration    C;
            if(Storage()->ConfigurationDB().GetRecord("id", UUID, C)) {
                Poco::JSON::Object  Answer;

                C.to_json(Answer);
                ReturnObject(Request, Answer, Response);
                return;
            }
            NotFound(Request, Response);
            return;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response, "Internal error. Consult documentation and try again.");
    }

    void RESTAPI_configurations_handler::DoDelete(Poco::Net::HTTPServerRequest &Request,
                                                 Poco::Net::HTTPServerResponse &Response) {
        try {
            auto UUID = GetBinding("uuid","");
            if(UUID.empty()) {
                BadRequest(Request, Response, "Missing UUID.");
                return;
            }

            ProvObjects::DeviceConfiguration    C;
            if(Storage()->ConfigurationDB().GetRecord("id", UUID, C)) {
                if(!C.inUse.empty()) {
                    BadRequest(Request, Response, "Configuration still in use.");
                    return;
                }

                if(Storage()->ConfigurationDB().DeleteRecord("id", UUID)) {
                    OK(Request, Response);
                    return;
                }
                BadRequest(Request,Response,"Internal error. Please try again");
                return;
            }
            NotFound(Request, Response);
            return;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response, "Internal error. Consult documentation and try again.");
    }

    void RESTAPI_configurations_handler::DoPost(Poco::Net::HTTPServerRequest &Request,
                                               Poco::Net::HTTPServerResponse &Response) {
        try {
            auto UUID = GetBinding("uuid","");
            if(UUID.empty()) {
                BadRequest(Request, Response, "Missing UUID.");
                return;
            }

            std::cout << __LINE__ << std::endl;

            ProvObjects::DeviceConfiguration C;
            Poco::JSON::Parser IncomingParser;
            Poco::JSON::Object::Ptr Obj = IncomingParser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
            if (!C.from_json(Obj)) {
                BadRequest(Request, Response, "Bad JSON Document posted.");
                return;
            }
            std::cout << __LINE__ << std::endl;

            if(C.info.name.empty()) {
                BadRequest(Request, Response, "Missing name.");
                return;
            }
            std::cout << __LINE__ << std::endl;

            if(!C.managementPolicy.empty() && !Storage()->PolicyDB().Exists("id",C.managementPolicy)) {
                BadRequest(Request, Response, "Unknown management policy.");
                return;
            }
            std::cout << __LINE__ << std::endl;

            C.info.modified = C.info.created = std::time(nullptr);
            C.info.id = Daemon()->CreateUUID();
            for(auto &i:C.info.notes)
                i.createdBy = UserInfo_.userinfo.email;
            std::cout << __LINE__ << std::endl;

            C.inUse.clear();
            if(C.deviceTypes.empty() || !Storage()->AreAcceptableDeviceTypes(C.deviceTypes, true)) {
                BadRequest(Request, Response, "Missing valid device types.");
                return;
            }
            std::cout << __LINE__ << std::endl;

            try {
                std::cout << __LINE__ << std::endl;
                for(const auto &i:C.configuration) {
                    Poco::JSON::Parser  P;
                    std::cout << "Config:>>>" << std::endl << i.configuration << std::endl << "<<<" << std::endl;
                    auto T = P.parse(i.configuration).extract<Poco::JSON::Object>();
                }
                std::cout << __LINE__ << std::endl;
            } catch (const Poco::Exception &E) {
                std::cout << __LINE__ << std::endl;
                BadRequest(Request, Response, "Invalid configuration portion.");
                return;
            }

            if(Storage()->ConfigurationDB().CreateRecord(C)) {
                std::cout << __LINE__ << std::endl;
                Storage()->ConfigurationDB().GetRecord("id", C.info.id, C);
                Poco::JSON::Object  Answer;
                std::cout << __LINE__ << std::endl;

                if(!C.managementPolicy.empty())
                    Storage()->PolicyDB().AddInUse("id",C.managementPolicy,Storage()->PolicyDB().Prefix(), C.info.id);

                std::cout << __LINE__ << std::endl;
                C.to_json(Answer);
                ReturnObject(Request, Answer, Response);
                std::cout << __LINE__ << std::endl;
                return;
            }
            std::cout << __LINE__ << std::endl;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response, "Internal error. Consult documentation and try again.");
    }

    void RESTAPI_configurations_handler::DoPut(Poco::Net::HTTPServerRequest &Request,
                                                Poco::Net::HTTPServerResponse &Response) {
        try {
            auto UUID = GetBinding("uuid","");
            if(UUID.empty()) {
                BadRequest(Request, Response, "Missing UUID.");
                return;
            }

            ProvObjects::DeviceConfiguration    Existing;
            if(!Storage()->ConfigurationDB().GetRecord("id", UUID, Existing)) {
                NotFound(Request, Response);
                return;
            }

            ProvObjects::DeviceConfiguration    NewConfig;
            Poco::JSON::Parser IncomingParser;
            Poco::JSON::Object::Ptr Obj = IncomingParser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
            if (!NewConfig.from_json(Obj)) {
                BadRequest(Request, Response, "Illegal JSON posted document.");
                return;
            }

            for(auto &i:NewConfig.info.notes)
                i.createdBy = UserInfo_.userinfo.email;

            if(NewConfig.managementPolicy.empty() || (NewConfig.managementPolicy!=Existing.managementPolicy && !Storage()->PolicyDB().Exists("id",NewConfig.managementPolicy))) {
                BadRequest(Request, Response, "Management policy is not valid.");
                return;
            }

            if(!NewConfig.deviceTypes.empty() && !Storage()->AreAcceptableDeviceTypes(NewConfig.deviceTypes, true)) {
                BadRequest(Request, Response, "Invalid device types.");
                return;
            }

            if(!NewConfig.deviceTypes.empty())
                Existing.deviceTypes = NewConfig.deviceTypes;

            if(!NewConfig.info.name.empty())
                Existing.info.name = NewConfig.info.name;

            if(!NewConfig.info.description.empty())
                Existing.info.description = NewConfig.info.description;

            NewConfig.info.modified = std::time(nullptr);

            if(!NewConfig.configuration.empty()) {
                try {
                    for(const auto &i:NewConfig.configuration) {
                        Poco::JSON::Parser  P;
                        auto T = P.parse(i.configuration).extract<Poco::JSON::Object>();
                    }
                } catch (const Poco::Exception &E) {
                    BadRequest(Request, Response, "Invalid configuration portion.");
                    return;
                }
            }

            if(!NewConfig.variables.empty())
                Existing.variables = NewConfig.variables;

            std::string OldPolicy;
            OldPolicy = Existing.managementPolicy;
            if(!NewConfig.managementPolicy.empty() && Existing.managementPolicy!=NewConfig.managementPolicy) {
                OldPolicy = Existing.managementPolicy;
                Existing.managementPolicy = NewConfig.managementPolicy;
            }

            if(Storage()->ConfigurationDB().UpdateRecord("id",UUID,Existing)) {
                if(!OldPolicy.empty()) {
                    Storage()->PolicyDB().DeleteInUse("id",OldPolicy,Storage()->ConfigurationDB().Prefix(),Existing.info.id);
                }

                if(!Existing.managementPolicy.empty()) {
                    Storage()->PolicyDB().AddInUse("id",Existing.managementPolicy,Storage()->ConfigurationDB().Prefix(),Existing.info.id);
                }

                ProvObjects::DeviceConfiguration    D;
                Storage()->ConfigurationDB().GetRecord("id",UUID,D);
                Poco::JSON::Object  Answer;
                D.to_json(Answer);
                ReturnObject(Request, Answer, Response);
                return;
            }
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response, "Internal error. Consult documentation and try again.");
    }
}