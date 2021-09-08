//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//


#include "RESTAPI_entity_handler.h"
#include "RESTAPI_ProvObjects.h"
#include "StorageService.h"

#include "Poco/JSON/Parser.h"
#include "Daemon.h"
#include "RESTAPI_SecurityObjects.h"
#include "RESTAPI_utils.h"

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

            if(!Storage()->EntityDB().GetRecord("id",UUID,E)) {
                NotFound(Request, Response);
                return;
            }

            if(!E.children.empty()) {
                BadRequest(Request, Response, "Entity still has children.");
                return;
            }

            if(Storage()->EntityDB().DeleteRecord("id",UUID)) {
                Storage()->EntityDB().DeleteChild("id",E.parent,UUID);
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
            std::string UUID = GetBinding("uuid", "");

            if(UUID.empty()) {
                BadRequest(Request, Response, "Missing UUID");
                return;
            }

            if(!Storage()->EntityDB().RootExists() && UUID != EntityDB::RootUUID()) {
                BadRequest(Request, Response, "Root entity must be created first.");
                return;
            }

            Poco::JSON::Parser IncomingParser;
            Poco::JSON::Object::Ptr Obj = IncomingParser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
            ProvObjects::Entity NewEntity;
            if (!NewEntity.from_json(Obj)) {
                BadRequest(Request, Response);
                return;
            }
            NewEntity.info.modified = NewEntity.info.created = std::time(nullptr);

            for(auto &i:NewEntity.info.notes)
                i.createdBy = UserInfo_.userinfo.email;

            //  When creating an entity, it cannot have any relations other that parent, notes, name, description. Everything else
            //  must be conveyed through PUT.
            NewEntity.info.id = (UUID==EntityDB::RootUUID()) ? UUID : Daemon()->CreateUUID() ;
            if(UUID==EntityDB::RootUUID())
                NewEntity.parent="";
            else if(NewEntity.parent.empty()) {
                BadRequest(Request, Response, "Parent UUID must be specified");
                return;
            } else {
                if(!Storage()->EntityDB().Exists("id",NewEntity.parent)) {
                    BadRequest(Request, Response, "Parent UUID must exist");
                    return;
                }
            }

            if(!NewEntity.deviceConfiguration.empty() && !Storage()->ConfigurationDB().Exists("id",NewEntity.deviceConfiguration)) {
                BadRequest(Request, Response, "Device Configuration does not exist");
                return;
            }

            NewEntity.venues.clear();
            NewEntity.children.clear();
            NewEntity.contacts.clear();
            NewEntity.locations.clear();

            if(Storage()->EntityDB().CreateShortCut(NewEntity)) {
                if(UUID==EntityDB::RootUUID())
                    Storage()->EntityDB().CheckForRoot();
                else {
                    Storage()->EntityDB().AddChild("id",NewEntity.parent,NewEntity.info.id);
                }

                if(!NewEntity.deviceConfiguration.empty())
                    Storage()->ConfigurationDB().AddInUse("id",NewEntity.deviceConfiguration,Storage()->EntityDB().Prefix(),NewEntity.info.id);

                Poco::JSON::Object  Answer;
                NewEntity.to_json(Answer);
                ReturnObject(Request, Answer, Response);
                return;
            }
            NotFound(Request,Response);
            return;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response);
    }

    /*
     * Put is a complex operation, it contains commands only.
     *      addContact=UUID, delContact=UUID,
     *      addLocation=UUID, delLocation=UUID,
     *      addVenue=UUID, delVenue=UUID,
     *      addEntity=UUID, delEntity=UUID
     *      addDevice=UUID, delDevice=UUID
     */

    void RESTAPI_entity_handler::DoPut(Poco::Net::HTTPServerRequest &Request,
                                                 Poco::Net::HTTPServerResponse &Response) {
        try {
            std::string UUID = GetBinding("uuid", "");

            if(UUID.empty()) {
                BadRequest(Request, Response, "Missing UUID");
                return;
            }

            ProvObjects::Entity Existing;
            if(!Storage()->EntityDB().GetRecord("id",UUID,Existing)) {
                NotFound(Request, Response);
                return;
            }

            Poco::JSON::Parser IncomingParser;
            auto RawObject = IncomingParser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
            ProvObjects::Entity NewEntity;
            if(!NewEntity.from_json(RawObject)) {
                BadRequest(Request, Response, "Cannot parse incoming JSON document.");
                return;
            }

            std::string OldConfiguration;
            if(!NewEntity.deviceConfiguration.empty() && !Storage()->ConfigurationDB().Exists("id",NewEntity.deviceConfiguration)) {
                BadRequest(Request, Response, "Device configuration does not exist");
                return;
            } else {
                OldConfiguration = Existing.deviceConfiguration;
                Existing.deviceConfiguration = NewEntity.deviceConfiguration;
            }

            for(auto &i:NewEntity.info.notes) {
                i.createdBy = UserInfo_.userinfo.email;
                Existing.info.notes.insert(Existing.info.notes.begin(),i);
            }

            if(!NewEntity.info.name.empty())
                Existing.info.name = NewEntity.info.name;
            if(!NewEntity.info.description.empty())
                Existing.info.description = NewEntity.info.description;

            Existing.info.modified = std::time(nullptr);

            std::string Error;
            if(!Storage()->Validate(Parameters_,Error)) {
                BadRequest(Request, Response, Error);
                return;
            }

            if(Storage()->EntityDB().UpdateRecord("id",UUID,Existing)) {
                for(const auto &i:Request) {
                    std::string Child{i.second};
                    auto UUID_parts = Utils::Split(Child,':');
                    if(i.first=="add" && UUID_parts[0] == "con") {
                        Storage()->EntityDB().AddContact("id", UUID, UUID_parts[1]);
                        Storage()->ContactDB().AddInUse("id",UUID_parts[1],Storage()->EntityDB().Prefix(), UUID);
                    } else if (i.first == "del" && UUID_parts[0] == "con") {
                        Storage()->EntityDB().DeleteContact("id", UUID, UUID_parts[1]);
                        Storage()->ContactDB().DeleteInUse("id",UUID_parts[1],Storage()->EntityDB().Prefix(),UUID);
                    } else if (i.first == "add" && UUID_parts[0] == "loc") {
                        Storage()->EntityDB().AddLocation("id", UUID, UUID_parts[1]);
                        Storage()->LocationDB().AddInUse("id",UUID_parts[1],Storage()->EntityDB().Prefix(),UUID);
                    } else if (i.first == "del" && UUID_parts[0] == "loc") {
                        Storage()->EntityDB().DeleteLocation("id", UUID, UUID_parts[1]);
                        Storage()->LocationDB().DeleteInUse("id",UUID_parts[1],Storage()->EntityDB().Prefix(),UUID);
                    }
                }

                if(!OldConfiguration.empty()) {
                    Storage()->ConfigurationDB().DeleteInUse("id", OldConfiguration, Storage()->EntityDB().Prefix(), Existing.info.id);
                }

                if(!NewEntity.deviceConfiguration.empty()) {
                    Storage()->ConfigurationDB().AddInUse("id", NewEntity.deviceConfiguration, Storage()->EntityDB().Prefix(), Existing.info.id);
                }

                Poco::JSON::Object  Answer;
                Storage()->EntityDB().GetRecord("id",UUID, Existing);
                Existing.to_json(Answer);
                ReturnObject(Request, Answer, Response);
                return;
            }
        } catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response);
    }
}