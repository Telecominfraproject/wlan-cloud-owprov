//
// Created by stephane bourque on 2022-02-23.
//

#include "RESTAPI_variables_handler.h"
#include "RESTAPI_db_helpers.h"

namespace OpenWifi {

    void RESTAPI_variables_handler::DoGet() {
        auto UUID = GetBinding("uuid","");
        if(UUID.empty()) {
            return BadRequest(RESTAPI::Errors::MissingUUID);
        }

        VariablesDB::RecordName Existing;
        if(!DB_.GetRecord("id",UUID,Existing)) {
            return NotFound();
        }

        Poco::JSON::Object Answer;
        if(QB_.AdditionalInfo)
            AddExtendedInfo(Existing, Answer);
        Existing.to_json(Answer);
        ReturnObject(Answer);
    }

    void RESTAPI_variables_handler::DoDelete() {
        auto UUID = GetBinding("uuid","");
        if(UUID.empty()) {
            return BadRequest(RESTAPI::Errors::MissingUUID);
        }

        VariablesDB::RecordName Existing;
        if(!DB_.GetRecord("id",UUID,Existing)) {
            return NotFound();
        }

        if(!Existing.configurations.empty()) {
            return BadRequest(RESTAPI::Errors::StillInUse);
        }
        RemoveMembership(StorageService()->VenueDB(),&ProvObjects::Venue::variables,Existing.venue,Existing.info.id);
        RemoveMembership(StorageService()->EntityDB(),&ProvObjects::Entity::variables,Existing.entity,Existing.info.id);

        DB_.DeleteRecord("id", UUID);
        return OK();
    }

    void RESTAPI_variables_handler::DoPost() {
        auto UUID = GetBinding("uuid","");
        if(UUID.empty()) {
            return BadRequest(RESTAPI::Errors::MissingUUID);
        }

        auto RawObj = ParseStream();
        VariablesDB::RecordName NewObj;
        if(!NewObj.from_json(RawObj)) {
            return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
        }

        if(!NewObj.entity.empty() && !StorageService()->EntityDB().Exists("id",NewObj.entity)) {
            return BadRequest(RESTAPI::Errors::EntityMustExist);
        }

        if(!NewObj.venue.empty() && !StorageService()->VenueDB().Exists("id",NewObj.venue)) {
            return BadRequest(RESTAPI::Errors::VenueMustExist);
        }

        if(!ProvObjects::CreateObjectInfo(RawObj,UserInfo_.userinfo,NewObj.info)) {
            return BadRequest((RESTAPI::Errors::MissingOrInvalidParameters));
        }

        if(DB_.CreateRecord(NewObj)) {
            AddMembership(StorageService()->VenueDB(),&ProvObjects::Venue::variables,NewObj.venue, NewObj.info.id);
            AddMembership(StorageService()->EntityDB(),&ProvObjects::Entity::variables,NewObj.entity, NewObj.info.id);

            VariablesDB::RecordName Added;
            StorageService()->VariablesDB().GetRecord("id",NewObj.info.id,Added);
            Poco::JSON::Object  Answer;
            Added.to_json(Answer);
            return ReturnObject(Answer);
        }
        return BadRequest(RESTAPI::Errors::RecordNotCreated);
    }

    void RESTAPI_variables_handler::DoPut() {
        auto UUID = GetBinding("uuid","");
        if(UUID.empty()) {
            return BadRequest(RESTAPI::Errors::MissingUUID);
        }

        VariablesDB::RecordName Existing;
        if(!StorageService()->VariablesDB().GetRecord("id",UUID,Existing)) {
            return NotFound();
        }

        auto RawObj = ParseStream();
        VariablesDB::RecordName NewObj;
        if(!NewObj.from_json(RawObj)) {
            return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
        }

        if(!ProvObjects::UpdateObjectInfo(RawObj,UserInfo_.userinfo,Existing.info)) {
            return BadRequest((RESTAPI::Errors::MissingOrInvalidParameters));
        }

        if(RawObj->has("variables"))
            Existing.variables = NewObj.variables;

        std::string ExistingEntity, MovingToEntity;
        if(RawObj->has("entity")) {
            ExistingEntity = Existing.entity;
            MovingToEntity = RawObj->get("entity").toString();
            if(!MovingToEntity.empty() && !StorageService()->EntityDB().Exists("id",MovingToEntity)) {
                return BadRequest(RESTAPI::Errors::EntityMustExist);
            }
            Existing.entity = MovingToEntity;
        }

        std::string ExistingVenue, MovingToVenue;
        if(RawObj->has("venue")) {
            ExistingVenue = Existing.venue;
            MovingToVenue = RawObj->get("venue").toString();
            if(!MovingToVenue.empty() && !StorageService()->VenueDB().Exists("id",MovingToVenue)) {
                return BadRequest(RESTAPI::Errors::VenueMustExist);
            }
            Existing.venue = MovingToVenue;
        }

        if(DB_.UpdateRecord("id", UUID, Existing)) {
            ManageMembership(StorageService()->VenueDB(), &ProvObjects::Venue::variables, ExistingVenue,
                             MovingToVenue, Existing.info.id);
            ManageMembership(StorageService()->EntityDB(), &ProvObjects::Entity::variables, ExistingEntity,
                             MovingToEntity, Existing.info.id);

            VariablesDB::RecordName Added;
            StorageService()->VariablesDB().GetRecord("id",NewObj.info.id,Added);
            Poco::JSON::Object  Answer;
            Added.to_json(Answer);
            return ReturnObject(Answer);
        }
        return BadRequest(RESTAPI::Errors::RecordNotCreated);
    }

}