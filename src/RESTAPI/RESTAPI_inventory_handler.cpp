//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//


#include "RESTAPI_inventory_handler.h"

#include "framework/RESTAPI_protocol.h"
#include "StorageService.h"
#include "APConfig.h"
#include "framework/RESTAPI_errors.h"
#include "AutoDiscovery.h"
#include "SDK_stubs.h"
#include "RESTAPI/RESTAPI_db_helpers.h"
#include "SerialNumberCache.h"

namespace OpenWifi{

    void GetRejectedLines(const Poco::JSON::Object::Ptr &Response, Types::StringVec & Warnings) {
        try {
            if(Response->has("results")) {
                auto Results = Response->get("results").extract<Poco::JSON::Object::Ptr>();
                auto Status = Results->get("status").extract<Poco::JSON::Object::Ptr>();
                auto Rejected = Status->getArray("rejected");
                for(const auto &i:*Rejected)
                    Warnings.push_back(i.toString());
            }
        } catch (...) {
        }
    }

    void RESTAPI_inventory_handler::DoGet() {
        ProvObjects::InventoryTag   Existing;
        std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER,"");
        if(SerialNumber.empty() || !DB_.GetRecord(RESTAPI::Protocol::SERIALNUMBER,SerialNumber,Existing)) {
            return NotFound();
        }

        Poco::JSON::Object  Answer;
        std::string Arg;
        if(HasParameter("config",Arg) && Arg=="true") {
            bool Explain = (HasParameter("explain",Arg) && Arg == "true");
            APConfig    Device(SerialNumber,Existing.deviceType,Logger_, Explain);

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
            std::string firmwareUpgrade = AutoDiscovery()->firmwareUpgrade();
            bool firmwareRCOnly = AutoDiscovery()->firmwareRCOnly();

            StorageService()->InventoryDB().FindFirmwareOptions(SerialNumber,firmwareUpgrade, firmwareRCOnly);

            Answer.set("firmwareUpgrade",firmwareUpgrade);
            Answer.set("firmwareRCOnly", firmwareRCOnly);
            return ReturnObject(Answer);
        } else if(HasParameter("applyConfiguration",Arg) && Arg=="true") {
            APConfig    Device(SerialNumber,Existing.deviceType,Logger_, false);
            Poco::JSON::Object::Ptr  Configuration;

            Types::StringVec Errors, Warnings;
            Poco::JSON::Object  ErrorsObj, WarningsObj;
            int ErrorCode;
            if(Device.Get(Configuration)) {
                Poco::JSON::Object::Ptr Response;
                if(SDK::SendConfigureCommand(SerialNumber,Configuration,Response)) {
                    std::ostringstream os;
                    Response->stringify(os);
                    // std::cout << "Success: " << os.str() << std::endl;
                    GetRejectedLines(Response, Warnings);
                    ErrorCode=0;
                } else {
                    std::ostringstream os;
                    Response->stringify(os);
                    ErrorCode=1;
                    // std::cout << "Failure: " << os.str() << std::endl;
                }
                Answer.set("appliedConfiguration", Configuration);
                Answer.set("response", Response);
            } else {
                Answer.set("appliedConfiguration", "");
                ErrorCode=1;
            }
            Answer.set("errorCode",ErrorCode);
            RESTAPI_utils::field_to_json(Answer,"errors", Errors);
            RESTAPI_utils::field_to_json(Answer,"warnings", Warnings);
            return ReturnObject(Answer);
        }  else if(QB_.AdditionalInfo) {
            AddExtendedInfo(Existing,Answer);
        }
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
            StorageService()->VenueDB().DeleteDevice("id",Existing.venue,Existing.info.id);

        if(!Existing.entity.empty())
            StorageService()->EntityDB().DeleteDevice("id",Existing.entity,Existing.info.id);

        if(!Existing.location.empty())
            StorageService()->LocationDB().DeleteInUse("id",Existing.location,DB_.Prefix(),Existing.info.id);

        if(!Existing.contact.empty())
            StorageService()->ContactDB().DeleteInUse("id",Existing.contact,DB_.Prefix(),Existing.info.id);

        if(!Existing.deviceConfiguration.empty())
            StorageService()->ConfigurationDB().DeleteInUse("id", Existing.deviceConfiguration, DB_.Prefix(), Existing.info.id);

        if(DB_.DeleteRecord("id", Existing.info.id)) {
            DB_.DeleteRecord(RESTAPI::Protocol::ID, Existing.info.id);
            SerialNumberCache()->DeleteSerialNumber(SerialNumber);
            return OK();
        }
        InternalError(RESTAPI::Errors::CouldNotBeDeleted);
    }

    void RESTAPI_inventory_handler::DoPost() {
        std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER,"");
        if(SerialNumber.empty()) {
            return BadRequest(RESTAPI::Errors::MissingSerialNumber);
        }

        if(!Utils::ValidSerialNumber(SerialNumber)) {
            return BadRequest(RESTAPI::Errors::InvalidSerialNumber);
        }

        Poco::toLowerInPlace(SerialNumber);
        if(DB_.Exists(RESTAPI::Protocol::SERIALNUMBER,SerialNumber)) {
            return BadRequest(RESTAPI::Errors::SerialNumberExists + " (" + SerialNumber + ")");
        }

        auto Obj = ParseStream();
        ProvObjects::InventoryTag NewObject;
        if (!NewObject.from_json(Obj)) {
            return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
        }

        if(NewObject.info.name.empty()) {
            return BadRequest( RESTAPI::Errors::NameMustBeSet);
        }

        if(NewObject.deviceType.empty() || !StorageService()->IsAcceptableDeviceType(NewObject.deviceType)) {
            return BadRequest(RESTAPI::Errors::InvalidDeviceTypes);
        }

        if(OpenWifi::EntityDB::IsRoot(NewObject.entity) || (!NewObject.entity.empty() && !StorageService()->EntityDB().Exists("id",NewObject.entity))) {
            return BadRequest(RESTAPI::Errors::ValidNonRootUUID);
        }

        if(!NewObject.venue.empty() && !StorageService()->VenueDB().Exists("id",NewObject.venue)) {
            return BadRequest(RESTAPI::Errors::VenueMustExist);
        }

        if(!NewObject.venue.empty() && !NewObject.entity.empty()) {
            return BadRequest(RESTAPI::Errors::NotBoth);
        }

        if(!NewObject.location.empty() && !StorageService()->LocationDB().Exists("id",NewObject.location)) {
            return BadRequest(RESTAPI::Errors::LocationMustExist);
        }

        if(!NewObject.contact.empty() && !StorageService()->ContactDB().Exists("id",NewObject.contact)) {
            return BadRequest(RESTAPI::Errors::ContactMustExist);
        }

        if(!NewObject.deviceConfiguration.empty() && !StorageService()->ConfigurationDB().Exists("id",NewObject.deviceConfiguration)) {
            return BadRequest(RESTAPI::Errors::ConfigurationMustExist);
        }

        if(!NewObject.managementPolicy.empty() && !StorageService()->PolicyDB().Exists("id",NewObject.managementPolicy)) {
            return BadRequest(RESTAPI::Errors::UnknownManagementPolicyUUID);
        }

        NewObject.info.modified = NewObject.info.created = std::time(nullptr);
        NewObject.info.id = MicroService::instance().CreateUUID();

        if(DB_.CreateRecord(NewObject)) {
            SerialNumberCache()->AddSerialNumber(SerialNumber);
            if (!NewObject.venue.empty())
                StorageService()->VenueDB().AddDevice("id",NewObject.venue,NewObject.info.id);
            if (!NewObject.entity.empty())
                StorageService()->EntityDB().AddDevice("id",NewObject.entity,NewObject.info.id);
            if (!NewObject.location.empty())
                StorageService()->LocationDB().AddInUse("id",NewObject.location,DB_.Prefix(),NewObject.info.id);
            if (!NewObject.contact.empty())
                StorageService()->ContactDB().AddInUse("id",NewObject.contact,DB_.Prefix(),NewObject.info.id);
            if (!NewObject.deviceConfiguration.empty())
                StorageService()->ConfigurationDB().AddInUse("id",NewObject.deviceConfiguration,DB_.Prefix(),NewObject.info.id);
            if (!NewObject.managementPolicy.empty())
                StorageService()->PolicyDB().AddInUse("id",NewObject.managementPolicy,DB_.Prefix(),NewObject.info.id);

            ProvObjects::InventoryTag   NewTag;
            DB_.GetRecord("id",NewObject.info.id,NewTag);
            Poco::JSON::Object Answer;
            NewTag.to_json(Answer);
            return ReturnObject(Answer);
        }
        InternalError(RESTAPI::Errors::RecordNotCreated);
    }

    void RESTAPI_inventory_handler::DoPut() {
        ProvObjects::InventoryTag   Existing;
        std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER,"");
        if(SerialNumber.empty() || !DB_.GetRecord(RESTAPI::Protocol::SERIALNUMBER,SerialNumber,Existing)) {
            return NotFound();
        }

        auto RawObject = ParseStream();
        ProvObjects::InventoryTag   NewObject;
        if(!NewObject.from_json(RawObject)) {
            return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
        }

        if(!NewObject.deviceType.empty()) {
            if(!StorageService()->IsAcceptableDeviceType(NewObject.deviceType)) {
                return BadRequest(RESTAPI::Errors::InvalidDeviceTypes);
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
            if(!NewVenue.empty() && !StorageService()->VenueDB().Exists("id",NewVenue)) {
                return BadRequest(RESTAPI::Errors::VenueMustExist);
            }
            MovingVenue = Existing.venue != NewVenue;
        }

        if(AssignIfPresent(RawObject, "entity",NewEntity)) {
            if(!NewEntity.empty() && !StorageService()->EntityDB().Exists("id",NewEntity)) {
                return BadRequest(RESTAPI::Errors::EntityMustExist);
            }
            MovingEntity = Existing.entity != NewEntity;
        }

        if(!NewEntity.empty() && !NewVenue.empty()) {
            return BadRequest(RESTAPI::Errors::NotBoth);
        }

        if(AssignIfPresent(RawObject, "location",NewLocation)) {
            if(!NewLocation.empty() && !StorageService()->LocationDB().Exists("id",NewLocation)) {
                return BadRequest(RESTAPI::Errors::LocationMustExist);
            }
            MovingLocation = Existing.location != NewLocation;
        }

        if(AssignIfPresent(RawObject, "contact",NewContact)) {
            if(!NewContact.empty() && !StorageService()->ContactDB().Exists("id",NewContact)) {
                return BadRequest(RESTAPI::Errors::ContactMustExist);
            }
            MovingContact = Existing.contact != NewContact;
        }

        if(AssignIfPresent(RawObject, "deviceConfiguration",NewConfiguration)) {
            if(!NewConfiguration.empty() && !StorageService()->ConfigurationDB().Exists("id",NewConfiguration)) {
                return BadRequest(RESTAPI::Errors::ConfigurationMustExist);
            }
            MovingConfiguration = Existing.deviceConfiguration != NewConfiguration;
        }

        if(AssignIfPresent(RawObject, "managementPolicy",NewPolicy)) {
            if(!NewPolicy.empty() && !StorageService()->PolicyDB().Exists("id",NewPolicy)) {
                return BadRequest(RESTAPI::Errors::UnknownManagementPolicyUUID);
            }
            MovingPolicy = Existing.managementPolicy != NewPolicy;
        }

        std::string Arg;
        bool UnAssign=false;
        if(HasParameter("unassign", Arg) && Arg=="true") {
            UnAssign=true;
            if(!Existing.venue.empty()) {
                StorageService()->VenueDB().DeleteDevice("id",Existing.venue,Existing.info.id);
            } else if(!Existing.entity.empty()) {
                StorageService()->EntityDB().DeleteDevice("id",Existing.entity,Existing.info.id);
            }
            if(!Existing.location.empty())
                StorageService()->LocationDB().DeleteInUse("id",Existing.location,DB_.Prefix(),Existing.info.id);
            if(!Existing.contact.empty())
                StorageService()->ContactDB().DeleteInUse("id",Existing.contact,DB_.Prefix(),Existing.info.id);
            if(!Existing.deviceConfiguration.empty())
                StorageService()->ConfigurationDB().DeleteInUse("id",Existing.deviceConfiguration,DB_.Prefix(),Existing.info.id);
            if(!Existing.managementPolicy.empty())
                StorageService()->PolicyDB().DeleteInUse("id",Existing.managementPolicy,DB_.Prefix(),Existing.info.id);
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

        if(StorageService()->InventoryDB().UpdateRecord("id", Existing.info.id, Existing)) {
            if(!UnAssign) {
                if(MovingEntity) {
                    if(!Existing.entity.empty())
                        StorageService()->EntityDB().DeleteDevice("id",Existing.entity,Existing.info.id);
                    if(!NewEntity.empty())
                        StorageService()->EntityDB().AddDevice("id", NewEntity, Existing.info.id);
                    Existing.entity = NewEntity;
                }
                if(MovingVenue) {
                    if(!Existing.venue.empty())
                        StorageService()->VenueDB().DeleteDevice("id",Existing.venue,Existing.info.id);
                    if(!NewVenue.empty())
                        StorageService()->VenueDB().AddDevice("id", NewVenue, Existing.info.id);
                    Existing.venue = NewVenue;
                }
                if(MovingConfiguration) {
                    if(!Existing.deviceConfiguration.empty())
                        StorageService()->ConfigurationDB().DeleteInUse("id",Existing.deviceConfiguration,DB_.Prefix(),Existing.info.id);
                    if(!NewConfiguration.empty())
                        StorageService()->ConfigurationDB().AddInUse("id",NewConfiguration,DB_.Prefix(),Existing.info.id);
                    Existing.deviceConfiguration = NewConfiguration;
                }
                if(MovingContact) {
                    if(!Existing.contact.empty())
                        StorageService()->ContactDB().DeleteInUse("id",Existing.contact,DB_.Prefix(),Existing.info.id);
                    if(!NewContact.empty())
                        StorageService()->ContactDB().AddInUse("id",NewContact,DB_.Prefix(),Existing.info.id);
                    Existing.contact = NewContact;
                }
                if(MovingLocation) {
                    if(!Existing.location.empty())
                        StorageService()->LocationDB().DeleteInUse("id",Existing.location,DB_.Prefix(),Existing.info.id);
                    if(!NewLocation.empty())
                        StorageService()->LocationDB().AddInUse("id",NewLocation,DB_.Prefix(),Existing.info.id);
                    Existing.location = NewLocation;
                }
                if(MovingPolicy) {
                    if(!Existing.managementPolicy.empty())
                        StorageService()->PolicyDB().DeleteInUse("id",Existing.managementPolicy,DB_.Prefix(),Existing.info.id);
                    if(!NewPolicy.empty())
                        StorageService()->PolicyDB().AddInUse("id",NewPolicy,DB_.Prefix(),Existing.info.id);
                    Existing.managementPolicy = NewPolicy;
                }
            }
            DB_.UpdateRecord("id", Existing.info.id, Existing);

            ProvObjects::InventoryTag   NewObjectCreated;
            DB_.GetRecord("id", Existing.info.id, NewObjectCreated);
            Poco::JSON::Object  Answer;
            NewObject.to_json(Answer);
            return ReturnObject(Answer);
        }
        InternalError(RESTAPI::Errors::RecordNotUpdated);
    }
}