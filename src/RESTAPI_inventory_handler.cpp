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
#include "APConfig.h"
#include "RESTAPI_errors.h"

namespace OpenWifi{
    void RESTAPI_inventory_handler::DoGet() {
        ProvObjects::InventoryTag   Existing;
        std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER,"");
        if(SerialNumber.empty() || !DB_.GetRecord(RESTAPI::Protocol::SERIALNUMBER,SerialNumber,Existing)) {
            NotFound();
            return;
        }

        std::string Arg;
        if(HasParameter("config",Arg) && Arg=="true") {
            APConfig    Device(SerialNumber,Existing.deviceType,Logger_);

            Poco::JSON::Object  Answer;
            Poco::JSON::Object::Ptr  Configuration;
            if(Device.Get(Configuration)) {
                Answer.set("config", Configuration);
            } else {
                Answer.set("config","none");
            }
            ReturnObject(Answer);
            return;
        }

        Poco::JSON::Object  Answer;
        Existing.to_json(Answer);
        ReturnObject(Answer);
    }

    void RESTAPI_inventory_handler::DoDelete() {
        ProvObjects::InventoryTag   Existing;
        std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER,"");
        if(SerialNumber.empty() || !DB_.GetRecord(RESTAPI::Protocol::SERIALNUMBER,SerialNumber,Existing)) {
            NotFound();
            return;
        }

        if(!Existing.venue.empty())
            Storage()->VenueDB().DeleteDevice("id",Existing.venue,Existing.info.id);

        if(!Existing.entity.empty())
            Storage()->EntityDB().DeleteDevice("id",Existing.entity,Existing.info.id);

        if(!Existing.location.empty())
            Storage()->LocationDB().DeleteInUse("id",Existing.location,DB_.Prefix(),Existing.info.id);

        if(!Existing.contact.empty())
            Storage()->ContactDB().DeleteInUse("id",Existing.contact,DB_.Prefix(),Existing.info.id);

        if(DB_.DeleteRecord("id", Existing.info.id)) {
            DB_.DeleteRecord(RESTAPI::Protocol::ID, Existing.info.id);
            OK();
        }
        BadRequest(RESTAPI::Errors::CouldNotBeDeleted);
    }

    void RESTAPI_inventory_handler::DoPost() {
        std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER,"");
        if(SerialNumber.empty()) {
            BadRequest(RESTAPI::Errors::MissingSerialNumber);
            return;
        }

        if(!Utils::ValidSerialNumber(SerialNumber)) {
            BadRequest(RESTAPI::Errors::InvalidSerialNumber);
            return;
        }

        Poco::toLowerInPlace(SerialNumber);
        if(DB_.Exists(RESTAPI::Protocol::SERIALNUMBER,SerialNumber)) {
            BadRequest(RESTAPI::Errors::SerialNumberExists + " (" + SerialNumber + ")");
            return;
        }

        auto Obj = ParseStream();
        ProvObjects::InventoryTag NewObject;
        if (!NewObject.from_json(Obj)) {
            BadRequest(RESTAPI::Errors::InvalidJSONDocument);
            return;
        }

        if(NewObject.info.name.empty()) {
            BadRequest( RESTAPI::Errors::NameMustBeSet);
            return;
        }

        if(NewObject.deviceType.empty() || !Storage()->IsAcceptableDeviceType(NewObject.deviceType)) {
            BadRequest(RESTAPI::Errors::InvalidDeviceTypes);
            return;
        }

        if(OpenWifi::EntityDB::IsRoot(NewObject.entity) || (!NewObject.entity.empty() && !Storage()->EntityDB().Exists("id",NewObject.entity))) {
            BadRequest(RESTAPI::Errors::ValidNonRootUUID);
            return;
        }

        if(!NewObject.venue.empty() && !Storage()->VenueDB().Exists("id",NewObject.venue)) {
            BadRequest(RESTAPI::Errors::VenueMustExist);
            return;
        }

        if(!NewObject.venue.empty() && !Storage()->VenueDB().Exists("id",NewObject.venue)) {
            BadRequest(RESTAPI::Errors::VenueMustExist);
            return;
        }

        NewObject.info.modified = NewObject.info.created = std::time(nullptr);
        NewObject.info.id = Daemon()->CreateUUID();

        if(DB_.CreateRecord(NewObject)) {
            if (!NewObject.venue.empty())
                Storage()->VenueDB().AddDevice("id",NewObject.venue,NewObject.info.id);
            if (!NewObject.entity.empty())
                Storage()->EntityDB().AddDevice("id",NewObject.entity,NewObject.info.id);
            if (!NewObject.location.empty())
                Storage()->LocationDB().AddInUse("id",NewObject.entity,DB_.Prefix(),NewObject.info.id);
            if (!NewObject.contact.empty())
                Storage()->ContactDB().AddInUse("id",NewObject.entity,DB_.Prefix(),NewObject.info.id);

            ProvObjects::InventoryTag   NewTag;
            DB_.GetRecord("id",NewObject.info.id,NewTag);
            Poco::JSON::Object Answer;
            NewTag.to_json(Answer);
            ReturnObject(Answer);
            return;
        }
        BadRequest(RESTAPI::Errors::RecordNotCreated);
    }

    void RESTAPI_inventory_handler::DoPut() {
        ProvObjects::InventoryTag   Existing;
        std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER,"");
        if(SerialNumber.empty() || !DB_.GetRecord(RESTAPI::Protocol::SERIALNUMBER,SerialNumber,Existing)) {
            NotFound();
            return;
        }

        auto RawObject = ParseStream();
        ProvObjects::InventoryTag   NewObject;
        if(!NewObject.from_json(RawObject)) {
            BadRequest(RESTAPI::Errors::InvalidJSONDocument);
            return;
        }

        if(!NewObject.deviceType.empty()) {
            if(!Storage()->IsAcceptableDeviceType(NewObject.deviceType)) {
                BadRequest(RESTAPI::Errors::InvalidDeviceTypes);
                return;
            }
        }

        for(auto &i:NewObject.info.notes) {
            i.createdBy = UserInfo_.userinfo.email;
            Existing.info.notes.insert(Existing.info.notes.begin(),i);
        }

        std::string NewVenue, NewEntity, NewLocation, NewContact;
        AssignIfPresent(RawObject, "venue",NewVenue);
        AssignIfPresent(RawObject, "entity",NewEntity);
        AssignIfPresent(RawObject, "location",NewLocation);
        AssignIfPresent(RawObject, "contact",NewContact);
        AssignIfPresent(RawObject, "rrm",Existing.rrm);

        if(!NewEntity.empty() && !NewVenue.empty()) {
            BadRequest(RESTAPI::Errors::NotBoth);
            return;
        }

        if(!NewVenue.empty() && !Storage()->VenueDB().Exists("id",NewVenue)) {
            BadRequest(RESTAPI::Errors::VenueMustExist);
            return;
        }

        if(!NewEntity.empty() && !Storage()->EntityDB().Exists("id",NewEntity)) {
            BadRequest(RESTAPI::Errors::EntityMustExist);
            return;
        }

        if(!NewLocation.empty() && !Storage()->LocationDB().Exists("id",NewLocation)) {
            BadRequest(RESTAPI::Errors::LocationMustExist);
            return;
        }

        if(!NewContact.empty() && !Storage()->ContactDB().Exists("id",NewContact)) {
            BadRequest(RESTAPI::Errors::ContactMustExist);
            return;
        }

        std::string Arg;
        bool UnAssign=false;
        if(HasParameter("unassign", Arg) && Arg=="true") {
            UnAssign=true;
            if(!Existing.venue.empty()) {
                Storage()->VenueDB().DeleteDevice("id",Existing.venue,Existing.info.id);
            } else if(!Existing.entity.empty()) {
                Storage()->EntityDB().DeleteDevice("id",Existing.entity,Existing.info.id);
            }
            Existing.venue.clear();
            Existing.entity.clear();
        }

        AssignIfPresent(RawObject, "name", Existing.info.name);
        AssignIfPresent(RawObject, "description", Existing.info.description);
        Existing.info.modified = std::time(nullptr);

        std::string NewDeviceConfiguration="1";
        AssignIfPresent(RawObject,"deviceConfiguration",NewDeviceConfiguration);
        if(NewDeviceConfiguration!="1") {
            if(NewDeviceConfiguration.empty()) {
                if(!Existing.deviceConfiguration.empty())
                    Storage()->ConfigurationDB().DeleteInUse("id",Existing.deviceConfiguration,DB_.Prefix(),Existing.info.id);
                Existing.deviceConfiguration.clear();
            } else if(NewDeviceConfiguration!=Existing.deviceConfiguration) {
                if(!Storage()->ConfigurationDB().Exists("id",NewDeviceConfiguration)) {
                    BadRequest(RESTAPI::Errors::ConfigurationMustExist);
                    return;
                }
                Storage()->ConfigurationDB().DeleteInUse("id",Existing.deviceConfiguration,DB_.Prefix(),Existing.info.id);
                Storage()->ConfigurationDB().AddInUse("id",NewDeviceConfiguration,DB_.Prefix(),Existing.info.id);
                Existing.deviceConfiguration=NewDeviceConfiguration;
            }
        }

        if(Storage()->InventoryDB().UpdateRecord("id", Existing.info.id, Existing)) {
            if(!UnAssign && !NewEntity.empty() && NewEntity!=Existing.entity) {
                Storage()->EntityDB().DeleteDevice("id",Existing.entity,Existing.info.id);
                Storage()->EntityDB().AddDevice("id",NewEntity,Existing.info.id);
                Existing.entity = NewEntity;
                Existing.venue.clear();
            } else if(!UnAssign && !NewVenue.empty() && NewVenue!=Existing.venue) {
                Storage()->VenueDB().DeleteDevice("id",Existing.venue,Existing.info.id);
                Storage()->VenueDB().AddDevice("id",NewVenue,Existing.info.id);
                Existing.entity.clear();
                Existing.venue = NewVenue;
            }

            if(NewContact!=Existing.contact) {
                if(!Existing.contact.empty())
                    Storage()->ContactDB().DeleteInUse("id", Existing.contact, DB_.Prefix(), Existing.info.id);
                if(!NewContact.empty())
                    Storage()->ContactDB().AddInUse("id", NewContact, DB_.Prefix(), Existing.info.id);
                Existing.contact = NewContact;
            }

            if(NewLocation!=Existing.location) {
                if(!Existing.location.empty())
                    Storage()->LocationDB().DeleteInUse("id", Existing.location, DB_.Prefix(), Existing.info.id);
                if(!NewLocation.empty())
                    Storage()->LocationDB().AddInUse("id", NewLocation, DB_.Prefix(), Existing.info.id);
                Existing.location = NewLocation;
            }

            DB_.UpdateRecord("id", Existing.info.id, Existing);

            ProvObjects::InventoryTag   NewObjectCreated;
            DB_.GetRecord("id", Existing.info.id, NewObjectCreated);
            Poco::JSON::Object  Answer;
            NewObject.to_json(Answer);
            ReturnObject(Answer);
            return;
        }

        BadRequest(RESTAPI::Errors::RecordNotUpdated);
    }
}