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

            if(IT.info.name.empty()) {
                BadRequest(Request, Response, "Name cannot be empty.");
                return;
            }

            if(IT.deviceType.empty() || !Storage()->IsAcceptableDeviceType(IT.deviceType)) {
                BadRequest(Request, Response, "DeviceType: '" + IT.deviceType + "' does not exist.");
                return;
            }

            if(IT.entity.empty() || OpenWifi::EntityDB::IsRoot(IT.entity) || !Storage()->EntityDB().Exists("id",IT.entity)) {
                BadRequest(Request, Response, "Device must be associated with a non-root and existing entity. UUID="+IT.entity);
                return;
            }

            if(!IT.venue.empty() && !Storage()->VenueDB().Exists("id",IT.venue)) {
                BadRequest(Request, Response, "Venue: " + IT.venue + " does not exist.");
                return;
            }

            IT.info.modified = IT.info.created = std::time(nullptr);
            IT.info.id = Daemon()->CreateUUID();

            if(Storage()->InventoryDB().CreateRecord(IT)) {
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

            if(RawObject->has("name") && !RawObject->get("name").toString().empty())
                ExistingObject.info.name = RawObject->get("name").toString();
            if(RawObject->has("description"))
                ExistingObject.info.description = RawObject->get("description").toString();
            ExistingObject.info.modified = std::time(nullptr);

            if(RawObject->has("deviceType")) {
                std::string DeviceType{RawObject->get("deviceType").toString()};
                if(!Storage()->IsAcceptableDeviceType(DeviceType)) {
                    BadRequest(Request, Response, "DeviceType: '" + DeviceType + "' does not exist.");
                    return;
                }
                ExistingObject.deviceType = DeviceType;
            }

            // if we are changing venues...
            if(RawObject->has("entity")) {
                std::string Entity{RawObject->get("entity").toString()};
                if(!Storage()->EntityDB().Exists("id",Entity)) {
                    BadRequest(Request, Response, "Entity association does not exist.");
                    return;
                }
                ExistingObject.entity = Entity;
            }

            if(RawObject->has("venue")) {
                std::string Venue{RawObject->get("venue").toString()};
                if(!Storage()->VenueDB().Exists("id",Venue)) {
                    BadRequest(Request, Response, "Venue association does not exist.");
                    return;
                }
                ExistingObject.venue = Venue;
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