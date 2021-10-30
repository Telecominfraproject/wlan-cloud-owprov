//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//


#include "RESTAPI_venue_handler.h"

#include "RESTObjects/RESTAPI_ProvObjects.h"
#include "StorageService.h"
#include "framework/RESTAPI_errors.h"
#include "RESTAPI/RESTAPI_db_helpers.h"

namespace OpenWifi{

    void RESTAPI_venue_handler::DoGet() {
        std::string UUID = GetBinding("uuid", "");
        ProvObjects::Venue Existing;
        if(UUID.empty() || !DB_.GetRecord("id",UUID,Existing)) {
            return NotFound();
        }

        Poco::JSON::Object Answer;
        if(QB_.AdditionalInfo)
            AddExtendedInfo(Existing, Answer);

        Existing.to_json(Answer);
        ReturnObject(Answer);
    }

    void RESTAPI_venue_handler::DoDelete() {
        std::string UUID = GetBinding("uuid", "");
        ProvObjects::Venue Existing;
        if(UUID.empty() || !DB_.GetRecord("id",UUID,Existing)) {
            return NotFound();
        }

        if(!Existing.children.empty() || !Existing.devices.empty()) {
            return BadRequest(RESTAPI::Errors::StillInUse);
        }

        if(!Existing.contact.empty())
            StorageService()->ContactDB().DeleteInUse("id",Existing.contact,StorageService()->VenueDB().Prefix(),UUID);
        if(!Existing.location.empty())
            StorageService()->LocationDB().DeleteInUse("id",Existing.location,StorageService()->VenueDB().Prefix(),UUID);
        if(!Existing.managementPolicy.empty())
            StorageService()->PolicyDB().DeleteInUse("id",Existing.managementPolicy,StorageService()->VenueDB().Prefix(),UUID);
        if(!Existing.deviceConfiguration.empty()) {
            for(auto &i:Existing.deviceConfiguration)
                StorageService()->ConfigurationDB().DeleteInUse("id",i,StorageService()->VenueDB().Prefix(),UUID);
        }
        if(!Existing.parent.empty())
            StorageService()->VenueDB().DeleteChild("id",Existing.parent,UUID);
        if(!Existing.entity.empty())
            StorageService()->EntityDB().DeleteVenue("id",Existing.entity,UUID);
        DB_.DeleteRecord("id",UUID);
        return OK();
    }

    void RESTAPI_venue_handler::DoPost() {
        std::string UUID = GetBinding("uuid", "");
        if(UUID.empty()) {
            return BadRequest(RESTAPI::Errors::MissingUUID);
        }

        auto Obj = ParseStream();
        ProvObjects::Venue NewObject;
        if (!NewObject.from_json(Obj)) {
            return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
        }

        if(!CreateObjectInfo(Obj, UserInfo_.userinfo, NewObject.info)) {
            return BadRequest( RESTAPI::Errors::NameMustBeSet);
        }

        if(NewObject.parent.empty() && NewObject.entity.empty()) {
            return BadRequest(RESTAPI::Errors::ParentOrEntityMustBeSet);
        }

        if(!NewObject.parent.empty() && !NewObject.entity.empty()) {
            return BadRequest(RESTAPI::Errors::NotBoth);
        }

        if(!NewObject.parent.empty() && !DB_.Exists("id",NewObject.parent)) {
            return BadRequest(RESTAPI::Errors::VenueMustExist);
        }

        if(NewObject.entity == EntityDB::RootUUID()) {
            return BadRequest(RESTAPI::Errors::ValidNonRootUUID);
        }

        if(!NewObject.entity.empty() && !StorageService()->EntityDB().Exists("id",NewObject.entity)) {
            return BadRequest(RESTAPI::Errors::EntityMustExist);
        }

        if(!NewObject.contact.empty() && !StorageService()->ContactDB().Exists("id",NewObject.contact)) {
            return BadRequest(RESTAPI::Errors::ContactMustExist);
        }

        if(!NewObject.location.empty() && !StorageService()->LocationDB().Exists("id",NewObject.location)) {
            return BadRequest(RESTAPI::Errors::LocationMustExist);
        }

        if(!NewObject.managementPolicy.empty() && !StorageService()->PolicyDB().Exists("id",NewObject.managementPolicy)) {
            return BadRequest(RESTAPI::Errors::UnknownManagementPolicyUUID);
        }

        if(!NewObject.deviceConfiguration.empty()) {
            for(auto &i:NewObject.deviceConfiguration) {
                if(!StorageService()->ConfigurationDB().Exists("id",i)) {
                    return BadRequest(RESTAPI::Errors::UnknownManagementPolicyUUID);
                }
            }
        }

        if(!NewObject.sourceIP.empty() && CIDR::ValidateIpRanges(NewObject.sourceIP)) {
            return BadRequest(RESTAPI::Errors::InvalidIPRanges);
        }

        NewObject.children.clear();

        if(DB_.CreateShortCut(NewObject)) {
            ProvObjects::Venue  NewRecord;
            DB_.GetRecord("id",NewObject.info.id,NewRecord);
            Poco::JSON::Object  Answer;
            NewRecord.to_json(Answer);
            return ReturnObject(Answer);
        }
        InternalError(RESTAPI::Errors::RecordNotCreated);
    }

    void RESTAPI_venue_handler::DoPut() {
        std::string UUID = GetBinding("uuid", "");
        ProvObjects::Venue Existing;
        if(UUID.empty() || !DB_.GetRecord("id",UUID,Existing)) {
            return NotFound();
        }

        auto RawObject = ParseStream();
        ProvObjects::Venue NewObject;
        if (!NewObject.from_json(RawObject)) {
            return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
        }

        if(!UpdateObjectInfo(RawObject, UserInfo_.userinfo, Existing.info)) {
            return BadRequest( RESTAPI::Errors::NameMustBeSet);
        }

        AssignIfPresent(RawObject, "rrm",Existing.rrm);

        if(RawObject->has("sourceIP")) {
            if(!NewObject.sourceIP.empty() && !CIDR::ValidateIpRanges(NewObject.sourceIP)) {
                return BadRequest(RESTAPI::Errors::InvalidIPRanges);
            }
            Existing.sourceIP = NewObject.sourceIP;
        }

        std::string MoveEntity;
        bool MovingEntity=false;
        if(AssignIfPresent(RawObject, "entity", MoveEntity)) {
            if(!MoveEntity.empty() && !StorageService()->EntityDB().Exists("id",MoveEntity)) {
                return BadRequest(RESTAPI::Errors::EntityMustExist);
            }
            MovingEntity = MoveEntity != Existing.entity;
        }

        std::string MoveVenue;
        bool MovingVenue=false;
        if(AssignIfPresent(RawObject, "venue", MoveVenue)) {
            if(!MoveVenue.empty() && !StorageService()->VenueDB().Exists("id",MoveVenue)) {
                return BadRequest(RESTAPI::Errors::VenueMustExist);
            }
            MovingVenue = MoveVenue != Existing.parent;
        }

        std::string MoveLocation;
        bool MovingLocation=false;
        if(AssignIfPresent(RawObject,"location",MoveLocation)) {
            if(!MoveLocation.empty() && !StorageService()->LocationDB().Exists("id",MoveLocation)) {
                return BadRequest(RESTAPI::Errors::LocationMustExist);
            }
            MovingLocation = MoveLocation!=Existing.location;
        }

        std::string MoveContact;
        bool MovingContact=false;
        if(AssignIfPresent(RawObject,"contact",MoveContact)) {
            if(!MoveContact.empty() && !StorageService()->ContactDB().Exists("id",MoveContact)) {
                return BadRequest(RESTAPI::Errors::ContactMustExist);
            }
            MovingContact = MoveContact!=Existing.contact;
        }

        std::string MovePolicy;
        bool MovingPolicy=false;
        if(AssignIfPresent(RawObject,"managementPolicy",MovePolicy)) {
            if(!MovePolicy.empty() && !StorageService()->PolicyDB().Exists("id",MovePolicy)) {
                return BadRequest(RESTAPI::Errors::UnknownManagementPolicyUUID);
            }
            MovingPolicy = MovePolicy != Existing.managementPolicy;
        }

        Types::UUIDvec_t MoveConfiguration;
        bool MovingConfiguration=false;
        if(RawObject->has("deviceConfiguration")){
            if(!NewObject.deviceConfiguration.empty()){
                for(auto &i:NewObject.deviceConfiguration) {
                    if(!StorageService()->ConfigurationDB().Exists("id",i)) {
                        return BadRequest(RESTAPI::Errors::ConfigurationMustExist);
                    }
                }
                MoveConfiguration = NewObject.deviceConfiguration;
             }
            MovingConfiguration = MoveConfiguration != Existing.deviceConfiguration;
        }

        if(StorageService()->VenueDB().UpdateRecord("id", UUID, Existing)) {
            if(MovingContact) {
                if(!Existing.contact.empty())
                    StorageService()->ContactDB().DeleteInUse("id",Existing.contact,DB_.Prefix(),Existing.info.id);
                if(!MoveContact.empty())
                    StorageService()->ContactDB().AddInUse("id", MoveContact, DB_.Prefix(), Existing.info.id);
                Existing.contact = MoveContact;
            }
            if(MovingEntity) {
                if(!Existing.entity.empty())
                    StorageService()->EntityDB().DeleteVenue("id", Existing.entity, Existing.info.id);
                if(!MoveEntity.empty())
                    StorageService()->EntityDB().AddVenue("id",MoveEntity,Existing.info.id);
                Existing.entity = MoveEntity;
            }
            if(MovingVenue) {
               if(!Existing.parent.empty())
                   DB_.DeleteChild("id",Existing.parent,Existing.info.id);
               if(!MoveVenue.empty())
                   DB_.AddChild("id", MoveVenue, Existing.info.id);
               Existing.parent = MoveVenue;
            }
            if(MovingLocation) {
                if(!Existing.location.empty())
                    StorageService()->LocationDB().DeleteInUse("id", Existing.location, DB_.Prefix(), Existing.info.id);
                if(!MoveLocation.empty())
                    StorageService()->LocationDB().AddInUse("id", MoveLocation, DB_.Prefix(), Existing.info.id);
                Existing.location = MoveLocation;
            }
            if(MovingPolicy) {
                if(!Existing.managementPolicy.empty())
                    StorageService()->PolicyDB().DeleteInUse("id", Existing.managementPolicy, DB_.Prefix(), Existing.info.id);
                if(!MovePolicy.empty())
                    StorageService()->PolicyDB().AddInUse("id", MovePolicy, DB_.Prefix(), Existing.info.id);
                Existing.managementPolicy = MovePolicy;
            }
            if(MovingConfiguration) {
                if(!Existing.deviceConfiguration.empty()) {
                    for(auto &i:Existing.deviceConfiguration)
                        StorageService()->ConfigurationDB().DeleteInUse("id", i, DB_.Prefix(), Existing.info.id);
                }
                if(!MoveConfiguration.empty()) {
                    for(auto &i:MoveConfiguration)
                        StorageService()->ConfigurationDB().AddInUse("id", i, DB_.Prefix(), Existing.info.id);
                }
                Existing.deviceConfiguration = MoveConfiguration;
            }

            DB_.UpdateRecord("id",Existing.info.id, Existing);
            ProvObjects::Venue AddedRecord;
            DB_.GetRecord("id",UUID,AddedRecord);
            Poco::JSON::Object  Answer;
            AddedRecord.to_json(Answer);
            return ReturnObject(Answer);
        }
        InternalError(RESTAPI::Errors::RecordNotUpdated);
    }
}