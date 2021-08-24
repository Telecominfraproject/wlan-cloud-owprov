//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
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

            if(!Utils::ValidSerialNumber(SerialNumber)) {
                BadRequest(Request, Response, "Invalid SerialNumber.");
                return;
            }

            Poco::toLowerInPlace(SerialNumber);

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

            if(OpenWifi::EntityDB::IsRoot(IT.entity) || (!IT.entity.empty() && !Storage()->EntityDB().Exists("id",IT.entity))) {
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
                if (!IT.venue.empty())
                    Storage()->VenueDB().AddDevice("id",IT.venue,IT.info.id);
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

            std::string Arg;
            if(HasParameter("unassign", Arg) && Arg=="true") {
                ExistingObject.entity.clear();
                if(!ExistingObject.venue.empty()) {
                    Storage()->VenueDB().DeleteDevice("id",ExistingObject.venue,ExistingObject.info.id);
                    ExistingObject.venue.clear();
                }
                if(!ExistingObject.deviceConfiguration.empty()) {
                    Storage()->ConfigurationDB().DeleteInUse("id",ExistingObject.deviceConfiguration, Storage()->InventoryDB().Prefix(), ExistingObject.info.id );
                    ExistingObject.deviceConfiguration.clear();
                }
                if(!ExistingObject.location.empty()) {
                    Storage()->LocationDB().DeleteInUse("id",ExistingObject.location,Storage()->InventoryDB().Prefix(), ExistingObject.info.id);
                    ExistingObject.location.clear();
                }
                if(!ExistingObject.contact.empty()) {
                    Storage()->ContactDB().DeleteInUse("id",ExistingObject.contact,Storage()->InventoryDB().Prefix(), ExistingObject.info.id);
                    ExistingObject.contact.clear();
                }
                ExistingObject.info.modified = std::time(nullptr);
                Storage()->InventoryDB().UpdateRecord("id", ExistingObject.info.id, ExistingObject);

                ProvObjects::InventoryTag   NewObject;
                Storage()->InventoryDB().GetRecord("id",ExistingObject.info.id,NewObject);
                Poco::JSON::Object  Answer;

                NewObject.to_json(Answer);
                ReturnObject(Request, Answer, Response);
                return;
            }

            AssignIfPresent(RawObject, "name", ExistingObject.info.name);
            AssignIfPresent(RawObject, "description", ExistingObject.info.description);
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
            std::string MoveToEntity;
            if(AssignIfPresent(RawObject,"entity",MoveToEntity)) {
                if(!Storage()->EntityDB().Exists("id",MoveToEntity)) {
                    BadRequest(Request, Response, "Entity association does not exist.");
                    return;
                }
            }

            std::string MoveToVenue;
            if(AssignIfPresent(RawObject, "venue", MoveToVenue)) {
                if(!Storage()->VenueDB().Exists("id",MoveToVenue)) {
                    BadRequest(Request, Response, "Venue association does not exist.");
                    return;
                }
            }

            if(Storage()->InventoryDB().UpdateRecord("id", ExistingObject.info.id, ExistingObject)) {

                if(!MoveToEntity.empty())
                    ExistingObject.entity = MoveToEntity;
                if(!MoveToVenue.empty()) {
                    if(!ExistingObject.venue.empty())
                        Storage()->VenueDB().DeleteDevice("id",ExistingObject.venue,ExistingObject.info.id);
                    Storage()->VenueDB().AddDevice("id",MoveToVenue,ExistingObject.info.id);
                    ExistingObject.venue = MoveToVenue;
                }
                Storage()->InventoryDB().UpdateRecord("id", ExistingObject.info.id, ExistingObject);

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