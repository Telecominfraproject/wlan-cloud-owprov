//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "RESTAPI_inventory_handler.h"

#include "StorageService.h"
#include "APConfig.h"
#include "AutoDiscovery.h"
#include "sdks/SDK_gw.h"
#include "sdks/SDK_sec.h"
#include "RESTAPI/RESTAPI_db_helpers.h"
#include "SerialNumberCache.h"
#include "DeviceTypeCache.h"

namespace OpenWifi{

    void GetRejectedLines(const Poco::JSON::Object::Ptr &Response, Types::StringVec & Warnings) {
        try {
            if(Response->has("results")) {
                auto Results = Response->get("results").extract<Poco::JSON::Object::Ptr>();
                auto Status = Results->get("status").extract<Poco::JSON::Object::Ptr>();
                auto Rejected = Status->getArray("rejected");
                std::transform(Rejected->begin(),Rejected->end(),std::back_inserter(Warnings), [](auto i) -> auto { return i.toString(); });
//                for(const auto &i:*Rejected)
  //                  Warnings.push_back(i.toString());
            }
        } catch (...) {
        }
    }

    void RESTAPI_inventory_handler::DoGet() {

        ProvObjects::InventoryTag   Existing;
        std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER,"");
        Logger().debug(Poco::format("%s: Retrieving inventory information.",SerialNumber));
        if(SerialNumber.empty() || !DB_.GetRecord(RESTAPI::Protocol::SERIALNUMBER,SerialNumber,Existing)) {
            return NotFound();
        }
        Logger().debug(Poco::format("%s,%s: Retrieving inventory information.", Existing.serialNumber, Existing.info.id ));

        Poco::JSON::Object  Answer;
        std::string Arg;
        if(HasParameter("config",Arg) && Arg=="true") {
            bool Explain = (HasParameter("explain",Arg) && Arg == "true");
            APConfig    Device(SerialNumber,Existing.deviceType,Logger(), Explain);

            auto Configuration = Poco::makeShared<Poco::JSON::Object>();
            if(Device.Get(Configuration)) {
                Answer.set("config", Configuration);
                if(Explain)
                    Answer.set("explanation", Device.Explanation());
            } else {
                Answer.set("config","none");
            }
            return ReturnObject(Answer);
        } else if(HasParameter("firmwareOptions", Arg) && Arg=="true") {
            ProvObjects::DeviceRules Rules;
            StorageService()->InventoryDB().EvaluateDeviceSerialNumberRules(SerialNumber,Rules);
            Answer.set("firmwareUpgrade",Rules.firmwareUpgrade);
            Answer.set("firmwareRCOnly", Rules.rcOnly == "yes" );
            return ReturnObject(Answer);
        } else if(HasParameter("applyConfiguration",Arg) && Arg=="true") {
            Logger().debug(Poco::format("%s: Retrieving configuration.",Existing.serialNumber));
            auto Device = std::make_shared<APConfig>(SerialNumber, Existing.deviceType, Logger(), false);
            auto Configuration = Poco::makeShared<Poco::JSON::Object>();
            Poco::JSON::Object ErrorsObj, WarningsObj;
            ProvObjects::InventoryConfigApplyResult Results;
            Logger().debug(Poco::format("%s: Computing configuration.",Existing.serialNumber));
            if (Device->Get(Configuration)) {
                std::ostringstream OS;
                Configuration->stringify(OS);
                Results.appliedConfiguration = OS.str();
                auto Response=Poco::makeShared<Poco::JSON::Object>();
                Logger().debug(Poco::format("%s: Sending configuration push.",Existing.serialNumber));
                if (SDK::GW::Device::Configure(this, SerialNumber, Configuration, Response)) {
                    Logger().debug(Poco::format("%s: Sending configuration pushed.",Existing.serialNumber));
                    GetRejectedLines(Response, Results.warnings);
                    Results.errorCode = 0;
                } else {
                    Logger().debug(Poco::format("%s: Sending configuration failed.",Existing.serialNumber));
                    Results.errorCode = 1;
                }
            } else {
                Logger().debug(Poco::format("%s: Configuration is bad.",Existing.serialNumber));
                Results.errorCode = 1;
            }
            Results.to_json(Answer);
            return ReturnObject(Answer);
        }   else if(QB_.AdditionalInfo) {
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

        MoveUsage(StorageService()->PolicyDB(),DB_,Existing.managementPolicy,"",Existing.info.id);
        RemoveMembership(StorageService()->VenueDB(),&ProvObjects::Venue::configurations,Existing.venue,Existing.info.id);
        RemoveMembership(StorageService()->EntityDB(),&ProvObjects::Entity::configurations,Existing.entity,Existing.info.id);
        MoveUsage(StorageService()->LocationDB(),DB_,Existing.location,"",Existing.info.id);
        MoveUsage(StorageService()->ContactDB(),DB_,Existing.contact,"",Existing.info.id);

        if(!Existing.deviceConfiguration.empty()) {
            ProvObjects::DeviceConfiguration    DC;
            if(StorageService()->ConfigurationDB().GetRecord("id", Existing.deviceConfiguration, DC)) {
                if(DC.subscriberOnly)
                    StorageService()->ConfigurationDB().DeleteRecord("id", Existing.deviceConfiguration);
                else
                    StorageService()->ConfigurationDB().DeleteInUse("id", Existing.deviceConfiguration, DB_.Prefix(),
                                                                    Existing.info.id);
            }
        }

        MoveUsage(StorageService()->PolicyDB(),DB_,Existing.managementPolicy,"",Existing.info.id);
        MoveUsage(StorageService()->LocationDB(),DB_,Existing.location,"",Existing.info.id);
        MoveUsage(StorageService()->ContactDB(),DB_,Existing.contact,"",Existing.info.id);
        MoveUsage(StorageService()->ConfigurationDB(),DB_,Existing.deviceConfiguration,"",Existing.info.id);
        ManageMembership(StorageService()->EntityDB(),&ProvObjects::Entity::devices,Existing.entity,"",Existing.info.id);
        ManageMembership(StorageService()->VenueDB(),&ProvObjects::Venue::devices,Existing.venue,"",Existing.info.id);
        DB_.DeleteRecord("id", Existing.info.id);
        SerialNumberCache()->DeleteSerialNumber(SerialNumber);
        return OK();
    }


    void RESTAPI_inventory_handler::DoPost() {
        std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER,"");
        if(SerialNumber.empty()) {
            return BadRequest(RESTAPI::Errors::MissingSerialNumber);
        }

        if(!NormalizeMac(SerialNumber)) {
            return BadRequest(RESTAPI::Errors::InvalidSerialNumber);
        }

        if(DB_.Exists(RESTAPI::Protocol::SERIALNUMBER,SerialNumber)) {
            return BadRequest(RESTAPI::Errors::SerialNumberExists);
        }

        const auto & RawObject = ParsedBody_;
        ProvObjects::InventoryTag NewObject;
        if (!NewObject.from_json(RawObject)) {
            return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
        }

        if((RawObject->has("deviceRules") && !ValidDeviceRules(NewObject.deviceRules,*this))) {
            return;
        }

        if(!Provisioning::DeviceClass::Validate(NewObject.devClass.c_str())) {
            return BadRequest(RESTAPI::Errors::InvalidDeviceClass);
        }

        if(NewObject.devClass.empty()) {
            NewObject.devClass = Provisioning::DeviceClass::ANY;
        }

        if(!ProvObjects::CreateObjectInfo(RawObject, UserInfo_.userinfo, NewObject.info)) {
            return BadRequest( RESTAPI::Errors::NameMustBeSet);
        }

        if(NewObject.deviceType.empty() || !DeviceTypeCache()->IsAcceptableDeviceType(NewObject.deviceType)) {
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

        RESTAPI::Errors::msg Error=RESTAPI::Errors::SUCCESS;
        auto ObjectsCreated = CreateObjects(NewObject,*this,Error);
        if(Error.err_num != 0) {
            return BadRequest(Error);
        }

        if(DB_.CreateRecord(NewObject)) {
            SDK::GW::Device::SetOwnerShip(this, SerialNumber, NewObject.entity, NewObject.venue, NewObject.subscriber);
            SerialNumberCache()->AddSerialNumber(SerialNumber,NewObject.deviceType);
            MoveUsage(StorageService()->PolicyDB(),DB_,"",NewObject.managementPolicy,NewObject.info.id);
            MoveUsage(StorageService()->LocationDB(),DB_,"",NewObject.location,NewObject.info.id);
            MoveUsage(StorageService()->ContactDB(),DB_,"",NewObject.contact,NewObject.info.id);
            MoveUsage(StorageService()->ConfigurationDB(),DB_,"",NewObject.deviceConfiguration,NewObject.info.id);
            ManageMembership(StorageService()->EntityDB(),&ProvObjects::Entity::devices,"",NewObject.entity,NewObject.info.id);
            ManageMembership(StorageService()->VenueDB(),&ProvObjects::Venue::devices,"",NewObject.venue,NewObject.info.id);

            ProvObjects::InventoryTag   NewTag;
            DB_.GetRecord("id",NewObject.info.id,NewTag);
            Poco::JSON::Object Answer;
            NewTag.to_json(Answer);
            return ReturnObject(Answer);
        }
        InternalError(RESTAPI::Errors::RecordNotCreated);
    }

    void RESTAPI_inventory_handler::DoPut() {

        std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER,"");
        if(SerialNumber.empty() || !Utils::ValidSerialNumber(SerialNumber)) {
            return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
        }

        ProvObjects::InventoryTag   Existing;
        if(SerialNumber.empty() || !DB_.GetRecord(RESTAPI::Protocol::SERIALNUMBER,SerialNumber,Existing)) {
            return NotFound();
        }

        auto RemoveSubscriber = GetParameter("removeSubscriber");
        if(!RemoveSubscriber.empty()) {
            if(Existing.subscriber == RemoveSubscriber) {
                Logger().information(Poco::format("%s: removing subscriber (%s)", SerialNumber, RemoveSubscriber));
                ProvObjects::DeviceConfiguration    DC;
                if(StorageService()->ConfigurationDB().GetRecord("id",Existing.deviceConfiguration,DC)) {
                    Logger().information(Poco::format("%s: removing configuration for subscriber (%s)", SerialNumber, RemoveSubscriber));
                    if(DC.subscriberOnly) {
                        if(!StorageService()->ConfigurationDB().DeleteRecord("id", Existing.deviceConfiguration)) {
                            Logger().debug("Could not delete the subscriber configuration");
                        }
                    }
                    else {
                        Logger().debug("Configurations is not for a subscriber.");
                    }
                    Existing.deviceConfiguration = "";
                }
                Existing.subscriber = "";
                Poco::JSON::Object state;
                state.set("date",OpenWifi::Now());
                state.set("method","auto-discovery");
                state.set("last-operation", "returned to inventory");
                std::ostringstream OO;
                state.stringify(OO);
                Existing.state = OO.str();
                StorageService()->InventoryDB().UpdateRecord("id",Existing.info.id,Existing);
                RemoveMembership(StorageService()->EntityDB(),&ProvObjects::Entity::devices,"id",Existing.info.id);
                Poco::JSON::Object  Answer;
                Existing.to_json(Answer);
                SDK::GW::Device::SetSubscriber(nullptr, SerialNumber, "");
                return ReturnObject(Answer);
            } else {
                Logger().information(Poco::format("%s: wrong subscriber (%s)", SerialNumber, RemoveSubscriber));
            }
            return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
        }

        const auto & RawObject = ParsedBody_;
        ProvObjects::InventoryTag   NewObject;
        if(!NewObject.from_json(RawObject)) {
            return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
        }

        if((RawObject->has("deviceRules") && !ValidDeviceRules(NewObject.deviceRules,*this))) {
            return;
        }

        if(!Provisioning::DeviceClass::Validate(NewObject.devClass.c_str())) {
            return BadRequest(RESTAPI::Errors::InvalidDeviceClass);
        }

        if(!NewObject.deviceType.empty()) {
            if(!DeviceTypeCache()->IsAcceptableDeviceType(NewObject.deviceType)) {
                return BadRequest(RESTAPI::Errors::InvalidDeviceTypes);
            }
        }

        if(!UpdateObjectInfo(RawObject, UserInfo_.userinfo, Existing.info)) {
            return BadRequest(RESTAPI::Errors::NameMustBeSet);
        }

        if(RawObject->has("deviceRules"))
            Existing.deviceRules = NewObject.deviceRules;

        std::string FromPolicy, ToPolicy;
        if(!CreateMove(RawObject,"managementPolicy",&InventoryDB::RecordName::managementPolicy, Existing, FromPolicy,
                       ToPolicy, StorageService()->PolicyDB()))
            return BadRequest(RESTAPI::Errors::EntityMustExist);

        std::string FromEntity, ToEntity;
        if(!CreateMove(RawObject,"entity",&InventoryDB::RecordName::entity, Existing, FromEntity, ToEntity,
                       StorageService()->EntityDB()))
            return BadRequest(RESTAPI::Errors::EntityMustExist);

        std::string FromVenue, ToVenue;
        if(!CreateMove(RawObject,"venue",&InventoryDB::RecordName::venue, Existing, FromVenue, ToVenue,
                       StorageService()->VenueDB()))
            return BadRequest(RESTAPI::Errors::VenueMustExist);

        std::string FromLocation, ToLocation;
        if(!CreateMove(RawObject,"location",&InventoryDB::RecordName::location, Existing, FromLocation, ToLocation,
                       StorageService()->LocationDB()))
            return BadRequest(RESTAPI::Errors::VenueMustExist);

        std::string FromContact, ToContact;
        if(!CreateMove(RawObject,"contact",&InventoryDB::RecordName::contact, Existing, FromContact, ToContact,
                       StorageService()->ContactDB()))
            return BadRequest(RESTAPI::Errors::VenueMustExist);

        std::string FromConfiguration, ToConfiguration;
        if(!CreateMove(RawObject,"deviceConfiguration",&InventoryDB::RecordName::deviceConfiguration, Existing,
                       FromConfiguration, ToConfiguration, StorageService()->ConfigurationDB()))
            return BadRequest(RESTAPI::Errors::ConfigurationMustExist);

        std::string NewSubScriber;
        if(AssignIfPresent(RawObject, "subscriber", NewSubScriber)) {
            if(!NewSubScriber.empty()) {
                if(NewSubScriber!=Existing.subscriber) {
                    SecurityObjects::UserInfo   U;
                    if(SDK::Sec::Subscriber::Get(this, NewSubScriber, U)) {
                        Existing.subscriber = NewSubScriber;
                    } else {
                        return BadRequest(RESTAPI::Errors::SubscriberMustExist);
                    }
                }
            } else {
                Existing.subscriber = "";
            }
        }

        if( RawObject->has("devClass") && NewObject.devClass!= Existing.devClass) {
            Existing.devClass = NewObject.devClass;
        }

        if( RawObject->has("state") && NewObject.state!= Existing.state) {
            Existing.state = NewObject.state;
        }

        RESTAPI::Errors::msg Error=RESTAPI::Errors::SUCCESS;
        auto ObjectsCreated = CreateObjects(NewObject,*this,Error);
        if(Error.err_num != 0) {
            return BadRequest(Error);
        }

        if(!ObjectsCreated.empty()) {
            auto it = ObjectsCreated.find("configuration");
            if(it!=ObjectsCreated.end()) {
                FromConfiguration="";
                ToConfiguration=it->second;
                Existing.deviceConfiguration=ToConfiguration;
            }
        }

        if(StorageService()->InventoryDB().UpdateRecord("id", Existing.info.id, Existing)) {
            MoveUsage(StorageService()->PolicyDB(),DB_,FromPolicy,ToPolicy,Existing.info.id);
            MoveUsage(StorageService()->LocationDB(),DB_,FromLocation,ToLocation,Existing.info.id);
            MoveUsage(StorageService()->ContactDB(),DB_,FromContact,ToContact,Existing.info.id);
            MoveUsage(StorageService()->ConfigurationDB(),DB_,FromConfiguration,ToConfiguration,Existing.info.id);
            ManageMembership(StorageService()->EntityDB(),&ProvObjects::Entity::devices,FromEntity,ToEntity,Existing.info.id);
            ManageMembership(StorageService()->VenueDB(),&ProvObjects::Venue::devices,FromVenue,ToVenue,Existing.info.id);

            SDK::GW::Device::SetOwnerShip(this, SerialNumber, Existing.entity, Existing.venue, Existing.subscriber);

            ProvObjects::InventoryTag   NewObjectCreated;
            DB_.GetRecord("id", Existing.info.id, NewObjectCreated);
            Poco::JSON::Object  Answer;
            NewObject.to_json(Answer);
            return ReturnObject(Answer);
        }
        InternalError(RESTAPI::Errors::RecordNotUpdated);
    }
}