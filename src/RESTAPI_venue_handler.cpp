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
        if(UUID.empty()) {
            BadRequest(RESTAPI::Errors::MissingUUID);
            return;
        }

        ProvObjects::Venue V;
        if(Storage()->VenueDB().GetRecord("id",UUID,V)) {
            Poco::JSON::Object Answer;
            V.to_json(Answer);
            ReturnObject(Answer);
            return;
        }
        NotFound();
    }

    void RESTAPI_venue_handler::DoDelete() {
        std::string UUID = GetBinding("uuid", "");
        if(UUID.empty()) {
            BadRequest(RESTAPI::Errors::MissingUUID);
            return;
        }

        ProvObjects::Venue V;
        if(Storage()->VenueDB().GetRecord("id",UUID,V)) {

            if(!V.children.empty() || !V.devices.empty()) {
                BadRequest(RESTAPI::Errors::StillInUse);
                return;
            }

            if(!V.contact.empty())
                Storage()->ContactDB().DeleteInUse("id",V.contact,Storage()->VenueDB().Prefix(),UUID);
            if(!V.location.empty())
                Storage()->LocationDB().DeleteInUse("id",V.location,Storage()->VenueDB().Prefix(),UUID);
            if(!V.managementPolicy.empty())
                Storage()->PolicyDB().DeleteInUse("id",V.managementPolicy,Storage()->VenueDB().Prefix(),UUID);
            if(!V.deviceConfiguration.empty())
                Storage()->ConfigurationDB().DeleteInUse("id",V.deviceConfiguration,Storage()->VenueDB().Prefix(),UUID);
            if(!V.parent.empty())
                Storage()->VenueDB().DeleteChild("id",V.parent,UUID);
            if(!V.entity.empty())
                Storage()->EntityDB().DeleteVenue("id",V.entity,UUID);
            Storage()->VenueDB().DeleteRecord("id",UUID);
            OK();
            return;
        }
        NotFound();
    }

    void RESTAPI_venue_handler::DoPost() {
        auto Obj = ParseStream();
        ProvObjects::Venue V;
        if (!V.from_json(Obj)) {
            BadRequest(RESTAPI::Errors::InvalidJSONDocument);
            return;
        }

        if(V.parent.empty() && V.entity.empty()) {
            BadRequest(RESTAPI::Errors::ParentOrEntityMustBeSet);
            return;
        }

        if(!V.parent.empty() && !V.entity.empty()) {
            BadRequest(RESTAPI::Errors::NotBoth);
            return;
        }

        if(!V.parent.empty() && !Storage()->VenueDB().Exists("id",V.parent)) {
            BadRequest(RESTAPI::Errors::VenueMustExist);
            return;
        }

        if(V.entity == EntityDB::RootUUID()) {
            BadRequest(RESTAPI::Errors::ValidNonRootUUID);
            return;
        }

        if(!V.entity.empty() && !Storage()->EntityDB().Exists("id",V.entity)) {
            BadRequest(RESTAPI::Errors::EntityMustExist);
            return;
        }

        if(!V.contact.empty() && !Storage()->ContactDB().Exists("id",V.contact)) {
            BadRequest(RESTAPI::Errors::ContactMustExist);
            return;
        }

        if(!V.location.empty() && !Storage()->LocationDB().Exists("id",V.location)) {
            BadRequest(RESTAPI::Errors::LocationMustExist);
            return;
        }

        if(!V.managementPolicy.empty() && !Storage()->PolicyDB().Exists("id",V.managementPolicy)) {
            BadRequest(RESTAPI::Errors::UnknownManagementPolicyUUID);
            return;
        }

        V.children.clear();
        V.info.modified = V.info.created = std::time(nullptr);
        V.info.id = Daemon()->CreateUUID() ;
        for(auto &i:V.info.notes)
            i.createdBy = UserInfo_.userinfo.email;

        if(Storage()->VenueDB().CreateShortCut(V)) {
            Poco::JSON::Object  Answer;
            V.to_json(Answer);
            ReturnObject(Answer);
            return;
        }
        NotFound();
    }

    void RESTAPI_venue_handler::DoPut() {
        std::string UUID = GetBinding("uuid", "");
        if(UUID.empty()) {
            BadRequest(RESTAPI::Errors::MissingUUID);
            return;
        }

        ProvObjects::Venue  ExistingVenue;
        if(!Storage()->VenueDB().GetRecord("id",UUID, ExistingVenue)) {
            NotFound();
            return;
        }

        auto RawObject = ParseStream();
        if(RawObject->has("notes")) {
            SecurityObjects::append_from_json(RawObject, UserInfo_.userinfo, ExistingVenue.info.notes);
        }
        AssignIfPresent(RawObject, "name", ExistingVenue.info.name);
        AssignIfPresent(RawObject, "description", ExistingVenue.info.description);

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

        if(Storage()->VenueDB().UpdateRecord("id", UUID, ExistingVenue)) {
            ProvObjects::Venue AddedRecord;
            Storage()->VenueDB().GetRecord("id",UUID,AddedRecord);
            Poco::JSON::Object  Answer;
            AddedRecord.to_json(Answer);
            ReturnObject(Answer);
            return;
        }
        NotFound();
    }
}