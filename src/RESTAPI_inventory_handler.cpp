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
        std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER,"");
        if(SerialNumber.empty()) {
            BadRequest(RESTAPI::Errors::MissingSerialNumber);
            return;
        }

        ProvObjects::InventoryTag   IT;
        if(Storage()->InventoryDB().GetRecord(RESTAPI::Protocol::SERIALNUMBER,SerialNumber,IT)) {
            std::string Arg;
            if(HasParameter("config",Arg) && Arg=="true") {
                APConfig    Device(SerialNumber,IT.deviceType,Logger_);

                Poco::JSON::Object  Answer;
                Poco::JSON::Object::Ptr  Configuration;
                if(Device.Get(Configuration)) {
                    Answer.set("config", Configuration);
                } else {
                    Answer.set("config","none");
                }
                ReturnObject(Answer);
                return;
            } else {
                Poco::JSON::Object  Answer;
                IT.to_json(Answer);
                ReturnObject(Answer);
                return;
            }
        } else {
            NotFound();
        }
    }

    void RESTAPI_inventory_handler::DoDelete() {
        std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER,"");
        if(SerialNumber.empty()) {
            BadRequest(RESTAPI::Errors::MissingSerialNumber);
            return;
        }
        ProvObjects::InventoryTag   IT;
        if(!Storage()->InventoryDB().GetRecord(RESTAPI::Protocol::SERIALNUMBER,SerialNumber,IT)) {
            NotFound();
            return;
        }
        Storage()->InventoryDB().DeleteRecord(RESTAPI::Protocol::ID, IT.info.id);
        OK();
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

        if(Storage()->InventoryDB().Exists(RESTAPI::Protocol::SERIALNUMBER,SerialNumber)) {
            BadRequest(RESTAPI::Errors::SerialNumberExists + " (" + SerialNumber + ")");
            return;
        }

        auto Obj = ParseStream();
        ProvObjects::InventoryTag IT;
        if (!IT.from_json(Obj)) {
            BadRequest(RESTAPI::Errors::InvalidJSONDocument);
            return;
        }

        if(IT.info.name.empty()) {
            BadRequest( RESTAPI::Errors::NameMustBeSet);
            return;
        }

        if(IT.deviceType.empty() || !Storage()->IsAcceptableDeviceType(IT.deviceType)) {
            BadRequest(RESTAPI::Errors::InvalidDeviceTypes);
            return;
        }

        if(OpenWifi::EntityDB::IsRoot(IT.entity) || (!IT.entity.empty() && !Storage()->EntityDB().Exists("id",IT.entity))) {
            BadRequest(RESTAPI::Errors::ValidNonRootUUID);
            return;
        }

        if(!IT.venue.empty() && !Storage()->VenueDB().Exists("id",IT.venue)) {
            BadRequest(RESTAPI::Errors::VenueMustExist);
            return;
        }

        IT.info.modified = IT.info.created = std::time(nullptr);
        IT.info.id = Daemon()->CreateUUID();

        if(Storage()->InventoryDB().CreateRecord(IT)) {
            if (!IT.venue.empty())
                Storage()->VenueDB().AddDevice("id",IT.venue,IT.info.id);
            Poco::JSON::Object Answer;
            IT.to_json(Answer);
            ReturnObject(Answer);
        } else {
            BadRequest(RESTAPI::Errors::RecordNotCreated);
        }
    }

    void RESTAPI_inventory_handler::DoPut() {
        std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER,"");
        if(SerialNumber.empty()) {
            BadRequest(RESTAPI::Errors::MissingSerialNumber);
            return;
        }

        ProvObjects::InventoryTag   ExistingObject;
        if(!Storage()->InventoryDB().GetRecord(RESTAPI::Protocol::SERIALNUMBER,SerialNumber,ExistingObject)) {
            NotFound();
            return;
        }

        auto RawObject = ParseStream();
        if(RawObject->has("notes")) {
            SecurityObjects::append_from_json(RawObject, UserInfo_.userinfo, ExistingObject.info.notes);
        }

        std::string NewVenue, NewEntity;
        AssignIfPresent(RawObject, "venue",NewVenue);
        AssignIfPresent(RawObject, "entity",NewEntity);

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

        if(RawObject->has("deviceType")) {
            std::string DeviceType{RawObject->get("deviceType").toString()};
            if(!Storage()->IsAcceptableDeviceType(DeviceType)) {
                BadRequest(RESTAPI::Errors::InvalidDeviceTypes);
                return;
            }
            ExistingObject.deviceType = DeviceType;
        }

        std::string Arg;
        bool UnAssign=false;
        if(HasParameter("unassign", Arg) && Arg=="true") {
            UnAssign=true;
            if(!ExistingObject.venue.empty()) {
                Storage()->VenueDB().DeleteDevice("id",ExistingObject.venue,ExistingObject.info.id);
            } else if(!ExistingObject.entity.empty()) {
                Storage()->EntityDB().DeleteDevice("id",ExistingObject.entity,ExistingObject.info.id);
            }
            ExistingObject.venue.clear();
            ExistingObject.entity.clear();
        }

        AssignIfPresent(RawObject, "name", ExistingObject.info.name);
        AssignIfPresent(RawObject, "description", ExistingObject.info.description);
        ExistingObject.info.modified = std::time(nullptr);

        std::string NewDeviceConfiguration="1";
        AssignIfPresent(RawObject,"deviceConfiguration",NewDeviceConfiguration);
        if(NewDeviceConfiguration!="1") {
            if(NewDeviceConfiguration.empty()) {
                if(!ExistingObject.deviceConfiguration.empty())
                    Storage()->ConfigurationDB().DeleteInUse("id",ExistingObject.deviceConfiguration,Storage()->InventoryDB().Prefix(),ExistingObject.info.id);
                ExistingObject.deviceConfiguration.clear();
            } else if(NewDeviceConfiguration!=ExistingObject.deviceConfiguration) {
                if(!Storage()->ConfigurationDB().Exists("id",NewDeviceConfiguration)) {
                    BadRequest(RESTAPI::Errors::ConfigurationMustExist);
                    return;
                }
                Storage()->ConfigurationDB().DeleteInUse("id",ExistingObject.deviceConfiguration,Storage()->InventoryDB().Prefix(),ExistingObject.info.id);
                Storage()->ConfigurationDB().AddInUse("id",NewDeviceConfiguration,Storage()->InventoryDB().Prefix(),ExistingObject.info.id);
                ExistingObject.deviceConfiguration=NewDeviceConfiguration;
            }
        }

        if(Storage()->InventoryDB().UpdateRecord("id", ExistingObject.info.id, ExistingObject)) {
            if(!UnAssign && !NewEntity.empty() && NewEntity!=ExistingObject.entity) {
                Storage()->EntityDB().DeleteDevice("id",ExistingObject.entity,ExistingObject.info.id);
                Storage()->EntityDB().AddDevice("id",NewEntity,ExistingObject.info.id);
                ExistingObject.entity = NewEntity;
                ExistingObject.venue.clear();
            } else if(!UnAssign && !NewVenue.empty() && NewVenue!=ExistingObject.venue) {
                Storage()->VenueDB().DeleteDevice("id",ExistingObject.venue,ExistingObject.info.id);
                Storage()->VenueDB().AddDevice("id",NewVenue,ExistingObject.info.id);
                ExistingObject.entity.clear();
                ExistingObject.venue = NewVenue;
            }
            Storage()->InventoryDB().UpdateRecord("id", ExistingObject.info.id, ExistingObject);
            ProvObjects::InventoryTag   NewObject;
            Storage()->InventoryDB().GetRecord("id", ExistingObject.info.id, NewObject);
            Poco::JSON::Object  Answer;
            NewObject.to_json(Answer);
            ReturnObject(Answer);
        } else {
            BadRequest(RESTAPI::Errors::RecordNotUpdated);
        }
    }
}