//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//


#include "RESTAPI_venue_handler.h"
#include "RESTAPI_ProvObjects.h"
#include "StorageService.h"
#include "Poco/JSON/Parser.h"
#include "Daemon.h"
#include "RESTAPI_errors.h"

namespace OpenWifi{

    void RESTAPI_venue_handler::DoGet() {
        std::string UUID = GetBinding("uuid", "");
        ProvObjects::Venue Existing;
        if(UUID.empty() || !DB_.GetRecord("id",UUID,Existing)) {
            NotFound();
            return;
        }
        Poco::JSON::Object Answer;
        Existing.to_json(Answer);
        ReturnObject(Answer);
    }

    void RESTAPI_venue_handler::DoDelete() {
        std::string UUID = GetBinding("uuid", "");
        ProvObjects::Venue Existing;
        if(UUID.empty() || !DB_.GetRecord("id",UUID,Existing)) {
            NotFound();
            return;
        }

        if(!Existing.children.empty() || !Existing.devices.empty()) {
            BadRequest(RESTAPI::Errors::StillInUse);
            return;
        }

        if(!Existing.contact.empty())
            Storage()->ContactDB().DeleteInUse("id",Existing.contact,Storage()->VenueDB().Prefix(),UUID);
        if(!Existing.location.empty())
            Storage()->LocationDB().DeleteInUse("id",Existing.location,Storage()->VenueDB().Prefix(),UUID);
        if(!Existing.managementPolicy.empty())
            Storage()->PolicyDB().DeleteInUse("id",Existing.managementPolicy,Storage()->VenueDB().Prefix(),UUID);
        if(!Existing.deviceConfiguration.empty())
            Storage()->ConfigurationDB().DeleteInUse("id",Existing.deviceConfiguration,Storage()->VenueDB().Prefix(),UUID);
        if(!Existing.parent.empty())
            Storage()->VenueDB().DeleteChild("id",Existing.parent,UUID);
        if(!Existing.entity.empty())
            Storage()->EntityDB().DeleteVenue("id",Existing.entity,UUID);
        DB_.DeleteRecord("id",UUID);
        OK();
    }

    void RESTAPI_venue_handler::DoPost() {
        std::string UUID = GetBinding("uuid", "");
        if(UUID.empty()) {
            BadRequest(RESTAPI::Errors::MissingUUID);
            return;
        }

        auto Obj = ParseStream();
        ProvObjects::Venue NewObject;
        if (!NewObject.from_json(Obj)) {
            BadRequest(RESTAPI::Errors::InvalidJSONDocument);
            return;
        }

        if(NewObject.parent.empty() && NewObject.entity.empty()) {
            BadRequest(RESTAPI::Errors::ParentOrEntityMustBeSet);
            return;
        }

        if(!NewObject.parent.empty() && !NewObject.entity.empty()) {
            BadRequest(RESTAPI::Errors::NotBoth);
            return;
        }

        if(!NewObject.parent.empty() && !DB_.Exists("id",NewObject.parent)) {
            BadRequest(RESTAPI::Errors::VenueMustExist);
            return;
        }

        if(NewObject.entity == EntityDB::RootUUID()) {
            BadRequest(RESTAPI::Errors::ValidNonRootUUID);
            return;
        }

        if(!NewObject.entity.empty() && !Storage()->EntityDB().Exists("id",NewObject.entity)) {
            BadRequest(RESTAPI::Errors::EntityMustExist);
            return;
        }

        if(!NewObject.contact.empty() && !Storage()->ContactDB().Exists("id",NewObject.contact)) {
            BadRequest(RESTAPI::Errors::ContactMustExist);
            return;
        }

        if(!NewObject.location.empty() && !Storage()->LocationDB().Exists("id",NewObject.location)) {
            BadRequest(RESTAPI::Errors::LocationMustExist);
            return;
        }

        if(!NewObject.managementPolicy.empty() && !Storage()->PolicyDB().Exists("id",NewObject.managementPolicy)) {
            BadRequest(RESTAPI::Errors::UnknownManagementPolicyUUID);
            return;
        }

        NewObject.children.clear();
        NewObject.info.modified = NewObject.info.created = std::time(nullptr);
        NewObject.info.id = Daemon()->CreateUUID() ;
        for(auto &i:NewObject.info.notes)
            i.createdBy = UserInfo_.userinfo.email;

        if(DB_.CreateShortCut(NewObject)) {
            if(!NewObject.entity.empty())
                Storage()->EntityDB().AddVenue("id", NewObject.entity, NewObject.info.id);
            if(!NewObject.parent.empty())
                DB_.AddChild("id", NewObject.parent, NewObject.info.id);
            if(!NewObject.managementPolicy.empty())
                Storage()->PolicyDB().AddInUse("id", NewObject.managementPolicy, DB_.Prefix(), NewObject.info.id);
            if(!NewObject.location.empty())
                Storage()->LocationDB().AddInUse("id", NewObject.location, DB_.Prefix(), NewObject.info.id);
            if(!NewObject.contact.empty())
                Storage()->ContactDB().AddInUse("id", NewObject.contact, DB_.Prefix(), NewObject.info.id);

            ProvObjects::Venue  NewRecord;
            DB_.GetRecord("id",NewObject.info.id,NewRecord);
            Poco::JSON::Object  Answer;
            NewRecord.to_json(Answer);
            ReturnObject(Answer);
            return;
        }
        BadRequest(RESTAPI::Errors::RecordNotCreated);
    }

    void RESTAPI_venue_handler::DoPut() {
        std::string UUID = GetBinding("uuid", "");
        ProvObjects::Venue Existing;
        if(UUID.empty() || !DB_.GetRecord("id",UUID,Existing)) {
            NotFound();
            return;
        }

        auto RawObject = ParseStream();
        ProvObjects::Venue NewObject;
        if (!NewObject.from_json(RawObject)) {
            BadRequest(RESTAPI::Errors::InvalidJSONDocument);
            return;
        }

        for(auto &i:NewObject.info.notes) {
            i.createdBy = UserInfo_.userinfo.email;
            Existing.info.notes.insert(Existing.info.notes.begin(),i);
        }

        AssignIfPresent(RawObject, "name", Existing.info.name);
        AssignIfPresent(RawObject, "description", Existing.info.description);
        AssignIfPresent(RawObject, "rrm",Existing.rrm);

        std::string MoveEntity;
        if(AssignIfPresent(RawObject, "entity", MoveEntity) && !Storage()->EntityDB().Exists("id",MoveEntity)) {
            BadRequest(RESTAPI::Errors::EntityMustExist);
            return;
        }

        std::string MoveVenue;
        if(AssignIfPresent(RawObject, "venue", MoveVenue) && !Storage()->VenueDB().Exists("id",MoveVenue)) {
            BadRequest(RESTAPI::Errors::VenueMustExist);
            return;
        }

        std::string MoveLocation;
        if(AssignIfPresent(RawObject,"location",MoveLocation) && !Storage()->LocationDB().Exists("id",MoveLocation)) {
            BadRequest(RESTAPI::Errors::LocationMustExist);
            return;
        }

        std::string MoveContact;
        if(AssignIfPresent(RawObject,"contact",MoveContact) && !Storage()->ContactDB().Exists("id",MoveContact)) {
            BadRequest(RESTAPI::Errors::ContactMustExist);
            return;
        }

        std::string MovePolicy;
        if(AssignIfPresent(RawObject,"managementPolicy",MoveContact) && !Storage()->PolicyDB().Exists("id",MovePolicy)) {
            BadRequest(RESTAPI::Errors::UnknownManagementPolicyUUID);
            return;
        }

        std::string MoveConfiguration;
        if(AssignIfPresent(RawObject,"deviceConfiguration",MoveConfiguration) && !Storage()->ConfigurationDB().Exists("id",MoveConfiguration)) {
            BadRequest(RESTAPI::Errors::DeviceConfigurationUUID);
            return;
        }

        if(Storage()->VenueDB().UpdateRecord("id", UUID, Existing)) {
            if(MoveContact != Existing.contact) {
                if(!Existing.contact.empty())
                    Storage()->ContactDB().DeleteInUse("id",Existing.contact,DB_.Prefix(),Existing.info.id);
                if(!MoveContact.empty())
                    Storage()->ContactDB().AddInUse("id", MoveContact, DB_.Prefix(), Existing.info.id);
                Existing.contact = MoveContact;
            }
            if(MoveEntity != Existing.entity) {
                if(!Existing.entity.empty())
                    Storage()->EntityDB().DeleteVenue("id", Existing.entity, Existing.info.id);
                if(!MoveEntity.empty())
                    Storage()->EntityDB().AddVenue("id",MoveEntity,Existing.info.id);
                Existing.entity = MoveEntity;
            }
            if(MoveVenue != Existing.parent) {
               if(!Existing.parent.empty())
                   DB_.DeleteChild("id",Existing.parent,Existing.info.id);
               if(!MoveVenue.empty())
                   DB_.AddChild("id", MoveVenue, Existing.info.id);
               Existing.parent = MoveVenue;
            }
            if(MoveLocation != Existing.location) {
                if(!Existing.location.empty())
                    Storage()->LocationDB().DeleteInUse("id", Existing.location, DB_.Prefix(), Existing.info.id);
                if(!MoveLocation.empty())
                    Storage()->LocationDB().AddInUse("id", MoveLocation, DB_.Prefix(), Existing.info.id);
                Existing.location = MoveLocation;
            }
            if(MovePolicy != Existing.managementPolicy) {
                if(!Existing.managementPolicy.empty())
                    Storage()->PolicyDB().DeleteInUse("id", Existing.managementPolicy, DB_.Prefix(), Existing.info.id);
                if(!MovePolicy.empty())
                    Storage()->PolicyDB().AddInUse("id", MovePolicy, DB_.Prefix(), Existing.info.id);
                Existing.managementPolicy = MovePolicy;
            }

            if(MoveConfiguration != Existing.deviceConfiguration) {
                if(!Existing.deviceConfiguration.empty())
                    Storage()->ConfigurationDB().DeleteInUse("id", Existing.deviceConfiguration, DB_.Prefix(), Existing.info.id);
                if(!MoveConfiguration.empty())
                    Storage()->ConfigurationDB().AddInUse("id", MoveConfiguration, DB_.Prefix(), Existing.info.id);
                Existing.deviceConfiguration = MoveConfiguration;
            }

            DB_.UpdateRecord("id",Existing.info.id, Existing);
            ProvObjects::Venue AddedRecord;
            DB_.GetRecord("id",UUID,AddedRecord);
            Poco::JSON::Object  Answer;
            AddedRecord.to_json(Answer);
            ReturnObject(Answer);
            return;
        }
        BadRequest(RESTAPI::Errors::RecordNotUpdated);
    }
}