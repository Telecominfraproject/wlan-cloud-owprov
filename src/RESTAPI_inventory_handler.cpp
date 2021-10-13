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
#include "AutoDiscovery.h"

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
            bool Explain = (HasParameter("explain",Arg) && Arg == "true");
            APConfig    Device(SerialNumber,Existing.deviceType,Logger_, Explain);

            Poco::JSON::Object       Answer;
            Poco::JSON::Object::Ptr  Configuration;
            if(Device.Get(Configuration)) {
                Answer.set("config", Configuration);
                if(Explain)
                    Answer.set("explanation", Device.Explanation());
            } else {
                Answer.set("config","none");
            }
            return ReturnObject(Answer);
        } else if(HasParameter("firmwareOptions", Arg) && Arg=="true") {
            Poco::JSON::Object  Answer;

            std::string firmwareUpgrade = AutoDiscovery()->firmwareUpgrade();
            bool firmwareRCOnly = AutoDiscovery()->firmwareRCOnly();

            Storage()->InventoryDB().FindFirmwareOptions(SerialNumber,firmwareUpgrade, firmwareRCOnly);

            Answer.set("firmwareUpgrade",firmwareUpgrade);
            Answer.set("firmwareRCOnly", firmwareRCOnly);
            return ReturnObject(Answer);
        } else if(HasParameter("applyConfiguration",Arg) && Arg=="true") {
            APConfig    Device(SerialNumber,Existing.deviceType,Logger_, false);

            Poco::JSON::Object       Answer;
            Poco::JSON::Object::Ptr  Configuration;

            Types::StringVec Errors, Warnings;
            Poco::JSON::Object  ErrorsObj, WarningsObj;
            int ErrorCode;
            if(Device.Get(Configuration)) {
                Answer.set("appliedConfiguration", Configuration);
                ErrorCode=0;
            } else {
                Answer.set("appliedConfiguration", "");
                ErrorCode=1;
            }
            Answer.set("errorCode",ErrorCode);
            RESTAPI_utils::field_to_json(Answer,"errors", Errors);
            RESTAPI_utils::field_to_json(Answer,"warnings", Warnings);
            return ReturnObject(Answer);
        }

        Poco::JSON::Object  Answer;
        Existing.to_json(Answer);
        ReturnObject(Answer);
    }

    void RESTAPI_inventory_handler::DoDelete() {
        ProvObjects::InventoryTag   Existing;
        std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER,"");
        if(SerialNumber.empty() || !DB_.GetRecord(RESTAPI::Protocol::SERIALNUMBER,SerialNumber,Existing)) {
            return NotFound();
        }

        if(!Existing.venue.empty())
            Storage()->VenueDB().DeleteDevice("id",Existing.venue,Existing.info.id);

        if(!Existing.entity.empty())
            Storage()->EntityDB().DeleteDevice("id",Existing.entity,Existing.info.id);

        if(!Existing.location.empty())
            Storage()->LocationDB().DeleteInUse("id",Existing.location,DB_.Prefix(),Existing.info.id);

        if(!Existing.contact.empty())
            Storage()->ContactDB().DeleteInUse("id",Existing.contact,DB_.Prefix(),Existing.info.id);

        if(!Existing.deviceConfiguration.empty())
            Storage()->ConfigurationDB().DeleteInUse("id", Existing.deviceConfiguration, DB_.Prefix(), Existing.info.id);

        if(DB_.DeleteRecord("id", Existing.info.id)) {
            DB_.DeleteRecord(RESTAPI::Protocol::ID, Existing.info.id);
            OK();
        }
        InternalError(RESTAPI::Errors::CouldNotBeDeleted);
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

        if(!NewObject.venue.empty() && !NewObject.entity.empty()) {
            BadRequest(RESTAPI::Errors::NotBoth);
            return;
        }

        if(!NewObject.location.empty() && !Storage()->LocationDB().Exists("id",NewObject.location)) {
            BadRequest(RESTAPI::Errors::LocationMustExist);
            return;
        }

        if(!NewObject.contact.empty() && !Storage()->ContactDB().Exists("id",NewObject.contact)) {
            BadRequest(RESTAPI::Errors::ContactMustExist);
            return;
        }

        if(!NewObject.deviceConfiguration.empty() && !Storage()->ConfigurationDB().Exists("id",NewObject.deviceConfiguration)) {
            BadRequest(RESTAPI::Errors::ConfigurationMustExist);
            return;
        }

        if(!NewObject.managementPolicy.empty() && !Storage()->PolicyDB().Exists("id",NewObject.managementPolicy)) {
            BadRequest(RESTAPI::Errors::UnknownManagementPolicyUUID);
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
                Storage()->LocationDB().AddInUse("id",NewObject.location,DB_.Prefix(),NewObject.info.id);
            if (!NewObject.contact.empty())
                Storage()->ContactDB().AddInUse("id",NewObject.contact,DB_.Prefix(),NewObject.info.id);
            if (!NewObject.deviceConfiguration.empty())
                Storage()->ConfigurationDB().AddInUse("id",NewObject.deviceConfiguration,DB_.Prefix(),NewObject.info.id);
            if (!NewObject.managementPolicy.empty())
                Storage()->PolicyDB().AddInUse("id",NewObject.managementPolicy,DB_.Prefix(),NewObject.info.id);

            ProvObjects::InventoryTag   NewTag;
            DB_.GetRecord("id",NewObject.info.id,NewTag);
            Poco::JSON::Object Answer;
            NewTag.to_json(Answer);
            ReturnObject(Answer);
            return;
        }
        InternalError(RESTAPI::Errors::RecordNotCreated);
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

        std::string NewVenue, NewEntity, NewLocation, NewContact, NewConfiguration, NewPolicy;
        bool    MovingVenue=false,
                MovingEntity=false,
                MovingLocation=false,
                MovingContact=false,
                MovingConfiguration=false,
                MovingPolicy=false;

        AssignIfPresent(RawObject, "rrm",Existing.rrm);

        if(AssignIfPresent(RawObject, "venue",NewVenue)) {
            if(!NewVenue.empty() && !Storage()->VenueDB().Exists("id",NewVenue)) {
                BadRequest(RESTAPI::Errors::VenueMustExist);
                return;
            }
            MovingVenue = Existing.venue != NewVenue;
        }

        if(AssignIfPresent(RawObject, "entity",NewEntity)) {
            if(!NewEntity.empty() && !Storage()->EntityDB().Exists("id",NewEntity)) {
                BadRequest(RESTAPI::Errors::EntityMustExist);
                return;
            }
            MovingEntity = Existing.entity != NewEntity;
        }

        if(!NewEntity.empty() && !NewVenue.empty()) {
            BadRequest(RESTAPI::Errors::NotBoth);
            return;
        }

        if(AssignIfPresent(RawObject, "location",NewLocation)) {
            if(!NewLocation.empty() && !Storage()->LocationDB().Exists("id",NewLocation)) {
                BadRequest(RESTAPI::Errors::LocationMustExist);
                return;
            }
            MovingLocation = Existing.location != NewLocation;
        }

        if(AssignIfPresent(RawObject, "contact",NewContact)) {
            if(!NewContact.empty() && !Storage()->ContactDB().Exists("id",NewContact)) {
                BadRequest(RESTAPI::Errors::ContactMustExist);
                return;
            }
            MovingContact = Existing.contact != NewContact;
        }

        if(AssignIfPresent(RawObject, "deviceConfiguration",NewConfiguration)) {
            if(!NewConfiguration.empty() && !Storage()->ConfigurationDB().Exists("id",NewConfiguration)) {
                BadRequest(RESTAPI::Errors::ConfigurationMustExist);
                return;
            }
            MovingConfiguration = Existing.deviceConfiguration != NewConfiguration;
        }

        if(AssignIfPresent(RawObject, "managementPolicy",NewPolicy)) {
            if(!NewPolicy.empty() && !Storage()->PolicyDB().Exists("id",NewPolicy)) {
                BadRequest(RESTAPI::Errors::UnknownManagementPolicyUUID);
                return;
            }
            MovingPolicy = Existing.managementPolicy != NewPolicy;
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
            if(!Existing.location.empty())
                Storage()->LocationDB().DeleteInUse("id",Existing.location,DB_.Prefix(),Existing.info.id);
            if(!Existing.contact.empty())
                Storage()->ContactDB().DeleteInUse("id",Existing.contact,DB_.Prefix(),Existing.info.id);
            if(!Existing.deviceConfiguration.empty())
                Storage()->ConfigurationDB().DeleteInUse("id",Existing.deviceConfiguration,DB_.Prefix(),Existing.info.id);
            if(!Existing.managementPolicy.empty())
                Storage()->PolicyDB().DeleteInUse("id",Existing.managementPolicy,DB_.Prefix(),Existing.info.id);
            Existing.venue.clear();
            Existing.entity.clear();
            Existing.deviceConfiguration.clear();
            Existing.contact.clear();
            Existing.location.clear();
            Existing.managementPolicy.clear();
        }

        AssignIfPresent(RawObject, "name", Existing.info.name);
        AssignIfPresent(RawObject, "description", Existing.info.description);
        Existing.info.modified = std::time(nullptr);

        if(Storage()->InventoryDB().UpdateRecord("id", Existing.info.id, Existing)) {
            if(!UnAssign) {
                if(MovingEntity) {
                    if(!Existing.entity.empty())
                        Storage()->EntityDB().DeleteDevice("id",Existing.entity,Existing.info.id);
                    if(!NewEntity.empty())
                        Storage()->EntityDB().AddDevice("id", NewEntity, Existing.info.id);
                    Existing.entity = NewEntity;
                }
                if(MovingVenue) {
                    if(!Existing.venue.empty())
                        Storage()->VenueDB().DeleteDevice("id",Existing.venue,Existing.info.id);
                    if(!NewVenue.empty())
                        Storage()->VenueDB().AddDevice("id", NewVenue, Existing.info.id);
                    Existing.venue = NewVenue;
                }
                if(MovingConfiguration) {
                    if(!Existing.deviceConfiguration.empty())
                        Storage()->ConfigurationDB().DeleteInUse("id",Existing.deviceConfiguration,DB_.Prefix(),Existing.info.id);
                    if(!NewConfiguration.empty())
                        Storage()->ConfigurationDB().AddInUse("id",NewConfiguration,DB_.Prefix(),Existing.info.id);
                    Existing.deviceConfiguration = NewConfiguration;
                }
                if(MovingContact) {
                    if(!Existing.contact.empty())
                        Storage()->ContactDB().DeleteInUse("id",Existing.contact,DB_.Prefix(),Existing.info.id);
                    if(!NewContact.empty())
                        Storage()->ContactDB().AddInUse("id",NewContact,DB_.Prefix(),Existing.info.id);
                    Existing.contact = NewContact;
                }
                if(MovingLocation) {
                    if(!Existing.location.empty())
                        Storage()->LocationDB().DeleteInUse("id",Existing.location,DB_.Prefix(),Existing.info.id);
                    if(!NewLocation.empty())
                        Storage()->LocationDB().AddInUse("id",NewLocation,DB_.Prefix(),Existing.info.id);
                    Existing.location = NewLocation;
                }
                if(MovingPolicy) {
                    if(!Existing.managementPolicy.empty())
                        Storage()->PolicyDB().DeleteInUse("id",Existing.managementPolicy,DB_.Prefix(),Existing.info.id);
                    if(!NewPolicy.empty())
                        Storage()->PolicyDB().AddInUse("id",NewPolicy,DB_.Prefix(),Existing.info.id);
                    Existing.managementPolicy = NewPolicy;
                }
            }
            DB_.UpdateRecord("id", Existing.info.id, Existing);

            ProvObjects::InventoryTag   NewObjectCreated;
            DB_.GetRecord("id", Existing.info.id, NewObjectCreated);
            Poco::JSON::Object  Answer;
            NewObject.to_json(Answer);
            ReturnObject(Answer);
            return;
        }
        InternalError(RESTAPI::Errors::RecordNotUpdated);
    }
}