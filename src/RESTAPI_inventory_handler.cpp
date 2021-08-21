//
// Created by stephane bourque on 2021-08-16.
//

#include "RESTAPI_inventory_handler.h"
#include "RESTAPI_protocol.h"
#include "StorageService.h"
#include "Poco/JSON/Parser.h"
#include "Daemon.h"
#include "RESTAPI_utils.h"

namespace OpenWifi{
    void RESTAPI_inventory_handler::handleRequest(Poco::Net::HTTPServerRequest &Request,
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

    void RESTAPI_inventory_handler::DoGet(Poco::Net::HTTPServerRequest &Request,
                                        Poco::Net::HTTPServerResponse &Response) {
        try {
            std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER,"");
            if(SerialNumber.empty()) {
                BadRequest(Request, Response, "Missing SerialNumber.");
                return;
            }

            ProvObjects::InventoryTag   IT;
            if(Storage()->InventoryDB().GetRecord(RESTAPI::Protocol::SERIALNUMBER,SerialNumber,IT)) {
                Poco::JSON::Object  Answer;
                IT.to_json(Answer);
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

    void RESTAPI_inventory_handler::DoDelete(Poco::Net::HTTPServerRequest &Request,
                                           Poco::Net::HTTPServerResponse &Response) {
        try {
            std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER,"");
            if(SerialNumber.empty()) {
                BadRequest(Request, Response, "Missing SerialNumber.");
                return;
            }

            ProvObjects::InventoryTag   IT;
            if(!Storage()->InventoryDB().GetRecord(RESTAPI::Protocol::SERIALNUMBER,SerialNumber,IT)) {
                NotFound(Request,Response);
                return;
            }

            if(!IT.entity.empty())
                Storage()->EntityDB().DeleteChild(RESTAPI::Protocol::ID,IT.entity,IT.info.id);
            if(!IT.subEntity.empty())
                Storage()->EntityDB().DeleteChild(RESTAPI::Protocol::ID,IT.subEntity,IT.info.id);
            if(!IT.venue.empty())
                Storage()->EntityDB().DeleteChild(RESTAPI::Protocol::ID,IT.venue,IT.info.id);
            if(!IT.subVenue.empty())
                Storage()->EntityDB().DeleteChild(RESTAPI::Protocol::ID,IT.subVenue,IT.info.id);

            Storage()->InventoryDB().DeleteRecord(RESTAPI::Protocol::ID, IT.info.id);
            OK(Request, Response);
            return;
        }  catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response);
    }

    void RESTAPI_inventory_handler::DoPost(Poco::Net::HTTPServerRequest &Request,
                                            Poco::Net::HTTPServerResponse &Response) {
        try {
            std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER,"");
            if(SerialNumber.empty()) {
                BadRequest(Request, Response, "Missing SerialNumber.");
                return;
            }

            if(Storage()->InventoryDB().Exists(RESTAPI::Protocol::SERIALNUMBER,SerialNumber)) {
                BadRequest(Request,Response, "SerialNumber: " + SerialNumber + " already exists.");
                return;
            }

            Poco::JSON::Parser IncomingParser;
            Poco::JSON::Object::Ptr Obj = IncomingParser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
            ProvObjects::InventoryTag IT;
            if (!IT.from_json(Obj)) {
                BadRequest(Request, Response, "Cannot parse incoming POST.");
                return;
            }

            if(IT.entity.empty() || OpenWifi::EntityDB::IsRoot(IT.entity) || !Storage()->InventoryDB().Exists("id",IT.entity)) {
                BadRequest(Request, Response, "Device must be associated with a non-root and existing entity. UUID="+IT.entity);
                return;
            }

            if(!IT.venue.empty() && !Storage()->VenueDB().Exists("id",IT.venue)) {
                BadRequest(Request, Response, "Venue: " + IT.venue + " does not exist.");
                return;
            }

            IT.info.modified = IT.info.created = std::time(nullptr);
            IT.subEntity = IT.subVenue = "";
            IT.info.id = Daemon()->CreateUUID();

            if(Storage()->InventoryDB().CreateRecord(IT)) {
                Storage()->EntityDB().AddChild("id",IT.entity,IT.info.id);
                if(!IT.venue.empty())
                    Storage()->VenueDB().AddChild("id",IT.venue,IT.info.id);
                Poco::JSON::Object Answer;
                IT.to_json(Answer);
                ReturnObject(Request, Answer, Response);
                return;
            }
            BadRequest(Request, Response, "Record could not be added.");
            return;
        }  catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response);
    }

    void RESTAPI_inventory_handler::DoPut(Poco::Net::HTTPServerRequest &Request,
                                        Poco::Net::HTTPServerResponse &Response) {
        try {
            std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER,"");
            if(SerialNumber.empty()) {
                BadRequest(Request, Response, "Missing SerialNumber.");
                return;
            }

            ProvObjects::InventoryTag   ExistingObject;
            if(!Storage()->InventoryDB().GetRecord(RESTAPI::Protocol::SERIALNUMBER,SerialNumber,ExistingObject)) {
                NotFound(Request, Response);
                return;
            }

            Poco::JSON::Parser IncomingParser;
            Poco::JSON::Object::Ptr RawObject = IncomingParser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
            if(RawObject->has("notes")) {
                SecurityObjects::append_from_json(RawObject, UserInfo_.userinfo, ExistingObject.info.notes);
            }

            if(RawObject->has("name"))
                ExistingObject.info.name = RawObject->get("name").toString();
            if(RawObject->has("description"))
                ExistingObject.info.description = RawObject->get("description").toString();
            ExistingObject.info.modified = std::time(nullptr);

            // if we are changing venues...
            if(RawObject->has("entity")) {
                std::string Entity{RawObject->get("entity").toString()};
                if(!Storage()->EntityDB().Exists("id",Entity)) {
                    BadRequest(Request, Response, "Entity association does not exist.");
                    return;
                }
                if(Entity!=ExistingObject.entity) {
                    Storage()->EntityDB().DeleteChild("id", ExistingObject.entity, ExistingObject.info.id);
                    Storage()->EntityDB().AddChild("id",Entity,ExistingObject.info.id);
                }
            }

            if(RawObject->has("subEntity")) {
                std::string subEntity{RawObject->get("subEntity").toString()};
                if(!Storage()->EntityDB().Exists("id",subEntity)) {
                    BadRequest(Request, Response, "subEntity association does not exist.");
                    return;
                }
                if(subEntity!=ExistingObject.entity) {
                    Storage()->EntityDB().DeleteChild("id", ExistingObject.subEntity, ExistingObject.info.id);
                    Storage()->EntityDB().AddChild("id",subEntity,ExistingObject.info.id);
                }
            }

            if(RawObject->has("venue")) {
                std::string Venue{RawObject->get("venue").toString()};
                if(!Storage()->VenueDB().Exists("id",Venue)) {
                    BadRequest(Request, Response, "Venue association does not exist.");
                    return;
                }
                if(Venue!=ExistingObject.venue) {
                    if(!ExistingObject.venue.empty())
                        Storage()->VenueDB().DeleteChild("id", ExistingObject.venue, ExistingObject.info.id);
                    Storage()->VenueDB().AddChild("id",Venue,ExistingObject.info.id);
                }
            }

            if(RawObject->has("subVenue")) {
                std::string subVenue{RawObject->get("subVenue").toString()};
                if(!Storage()->VenueDB().Exists("id",subVenue)) {
                    BadRequest(Request, Response, "Venue association does not exist.");
                    return;
                }
                if(subVenue!=ExistingObject.subVenue) {
                    if(!ExistingObject.subVenue.empty())
                        Storage()->VenueDB().DeleteChild("id", ExistingObject.subVenue, ExistingObject.info.id);
                    Storage()->VenueDB().AddChild("id",subVenue,ExistingObject.info.id);
                }
            }

            if(RawObject->has("deviceType")) {
                std::string DeviceType{RawObject->get("deviceType").toString()};
                ExistingObject.deviceType = DeviceType;
            }

            if(Storage()->InventoryDB().UpdateRecord("id", ExistingObject.info.id, ExistingObject)) {
                Poco::JSON::Object  Answer;
                ExistingObject.to_json(Answer);
                ReturnObject(Request, Answer, Response);
                return;
            }
        }  catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest(Request, Response);
    }
}