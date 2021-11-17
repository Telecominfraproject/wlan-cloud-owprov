//
// Created by stephane bourque on 2021-11-09.
//

#include "RESTAPI_map_handler.h"

#include "framework/RESTAPI_protocol.h"
#include "RESTObjects/RESTAPI_ProvObjects.h"
#include "StorageService.h"
#include "Poco/JSON/Parser.h"
#include "Poco/StringTokenizer.h"
#include "framework/RESTAPI_errors.h"
#include "RESTAPI/RESTAPI_db_helpers.h"

namespace OpenWifi{

    void RESTAPI_map_handler::DoGet() {

        ProvObjects::Map   Existing;
        std::string UUID = GetBinding(RESTAPI::Protocol::UUID,"");
        if(UUID.empty() || !DB_.GetRecord(RESTAPI::Protocol::ID,UUID,Existing)) {
            return NotFound();
        }

        Poco::JSON::Object  Answer;
        if(QB_.AdditionalInfo)
            AddExtendedInfo(Existing,Answer);
        Existing.to_json(Answer);
        ReturnObject(Answer);
    }

    void RESTAPI_map_handler::DoDelete() {
        ProvObjects::Map   Existing;
        std::string UUID = GetBinding(RESTAPI::Protocol::UUID,"");
        if(UUID.empty() || !DB_.GetRecord(RESTAPI::Protocol::ID,UUID,Existing)) {
            return NotFound();
        }

        if(UserInfo_.userinfo.email!=Existing.creator) {
            return UnAuthorized("You must be the creator of the map to delete it");
        }

        if(DB_.DeleteRecord("id", Existing.info.id)) {
            return OK();
        }
        InternalError(RESTAPI::Errors::CouldNotBeDeleted);
    }

    static auto ValidateVisibility(const std::string &V) {
        return (V=="private" || V=="public" || V=="select");
    }

    void RESTAPI_map_handler::DoPost() {
        std::string UUID = GetBinding(RESTAPI::Protocol::UUID,"");
        if(UUID.empty()) {
            return BadRequest(RESTAPI::Errors::MissingUUID);
        }

        auto Obj = ParseStream();
        ProvObjects::Map NewObject;
        if (!NewObject.from_json(Obj)) {
            return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
        }

        if(!CreateObjectInfo(Obj, UserInfo_.userinfo, NewObject.info)) {
            return BadRequest( RESTAPI::Errors::NameMustBeSet);
        }

        NewObject.creator = UserInfo_.userinfo.Id;

        if(DB_.CreateRecord(NewObject)) {

            Poco::JSON::Object  Answer;
            ProvObjects::Map    M;
            DB_.GetRecord("id", NewObject.info.id,M);
            M.to_json(Answer);
            return ReturnObject(Answer);
        }
        InternalError(RESTAPI::Errors::RecordNotCreated);
    }

    void RESTAPI_map_handler::DoPut() {
        ProvObjects::Map   Existing;
        std::string UUID = GetBinding(RESTAPI::Protocol::UUID,"");
        if(UUID.empty() || !DB_.GetRecord(RESTAPI::Protocol::ID,UUID,Existing)) {
            return NotFound();
        }

        auto RawObject = ParseStream();
        ProvObjects::Map NewObject;
        if(!NewObject.from_json(RawObject)) {
            return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
        }

        if(!UpdateObjectInfo(RawObject, UserInfo_.userinfo, Existing.info)) {
            return BadRequest( RESTAPI::Errors::NameMustBeSet);
        }

        if(Existing.creator != UserInfo_.userinfo.Id) {
            if(Existing.visibility == ProvObjects::PRIVATE) {
                return UnAuthorized(RESTAPI::Errors::InsufficientAccessRights, ACCESS_DENIED);
            }
            if(Existing.visibility == ProvObjects::SELECT) {
                for(const auto &i:Existing.access.list) {
                    for(const auto &j:i.users.list) {
                        if(j==UserInfo_.userinfo.Id) {
                        }
                    }
                }
            }
        }

        if(RawObject->has("entity") && !StorageService()->EntityDB().Exists("id",NewObject.entity)) {
            return BadRequest(RESTAPI::Errors::EntityMustExist);
        }

        AssignIfPresent(RawObject,"entity",Existing.entity);
        AssignIfPresent(RawObject,"data", Existing.data);
        if(RawObject->has("visibility"))
            Existing.visibility = NewObject.visibility;

        if(DB_.UpdateRecord("id",UUID,Existing)) {

            ProvObjects::Map NewRecord;

            DB_.GetRecord("id", UUID, NewRecord);
            Poco::JSON::Object  Answer;
            NewRecord.to_json(Answer);
            return ReturnObject(Answer);
        }
        InternalError(RESTAPI::Errors::RecordNotUpdated);
    }
}