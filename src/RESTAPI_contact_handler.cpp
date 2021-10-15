//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "RESTAPI_contact_handler.h"
#include "RESTAPI_protocol.h"
#include "RESTAPI_ProvObjects.h"
#include "StorageService.h"
#include "Poco/JSON/Parser.h"
#include "Daemon.h"
#include "RESTAPI_errors.h"

namespace OpenWifi{
    void RESTAPI_contact_handler::DoGet() {

        std::string UUID = GetBinding("uuid","");
        ProvObjects::Contact   Existing;
        if(UUID.empty() || !DB_.GetRecord("id", UUID, Existing)) {
            return NotFound();
        }

        Poco::JSON::Object  Answer;
        std::string Arg;

        if(HasParameter("expandInUse",Arg) && Arg=="true") {
            Storage::ExpandedListMap    M;
            std::vector<std::string>    Errors;
            Poco::JSON::Object  Inner;
            if(Storage()->ExpandInUse(Existing.inUse,M,Errors)) {
                for(const auto &[type,list]:M) {
                    Poco::JSON::Array   ObjList;
                    for(const auto &i:list.entries) {
                        Poco::JSON::Object  O;
                        i.to_json(O);
                        ObjList.add(O);
                    }
                    Inner.set(type,ObjList);
                }
            }
            Answer.set("entries", Inner);
            return ReturnObject(Answer);
        } else if(HasParameter("withExtendedInfo",Arg) && Arg=="true") {
            Poco::JSON::Object  EI;
            if(!Existing.entity.empty()) {
                Poco::JSON::Object  EntObj;
                ProvObjects::Entity Entity;
                if(Storage()->EntityDB().GetRecord("id",Existing.entity,Entity)) {
                    Entity.to_json(EntObj);
                }
                EI.set("entity",EntObj);
            }

            if(!Existing.managementPolicy.empty()) {
                Poco::JSON::Object  PolObj;
                ProvObjects::ManagementPolicy Policy;
                if(Storage()->PolicyDB().GetRecord("id",Existing.managementPolicy,Policy)) {
                    Policy.to_json(PolObj);
                }
                EI.set("managementPolicy",PolObj);
            }
            Answer.set("extendedInfo", EI);
        }

        Existing.to_json(Answer);
        ReturnObject(Answer);
    }

    void RESTAPI_contact_handler::DoDelete() {

        std::string UUID = GetBinding("uuid","");
        ProvObjects::Contact   Existing;
        if(UUID.empty() || !DB_.GetRecord("id", UUID, Existing)) {
            NotFound();
            return;
        }

        bool Force=false;
        std::string Arg;
        if(HasParameter("force",Arg) && Arg=="true")
            Force=true;

        if(!Force && !Existing.inUse.empty()) {
            BadRequest(RESTAPI::Errors::StillInUse);
            return;
        }

        if(DB_.DeleteRecord("id",UUID)) {
            OK();
            return;
        }
        InternalError(RESTAPI::Errors::CouldNotBeDeleted);
    }

    void RESTAPI_contact_handler::DoPost() {
        std::string UUID = GetBinding(RESTAPI::Protocol::UUID,"");

        if(UUID.empty()) {
            BadRequest(RESTAPI::Errors::MissingUUID);
            return;
        }

        auto Obj = ParseStream();
        ProvObjects::Contact NewObject;
        if (!NewObject.from_json(Obj)) {
            BadRequest(RESTAPI::Errors::InvalidJSONDocument);
            return;
        }

        if(NewObject.entity.empty() || !Storage()->EntityDB().Exists("id",NewObject.entity)) {
            BadRequest(RESTAPI::Errors::EntityMustExist);
            return;
        }

        if(!NewObject.managementPolicy.empty() && !Storage()->PolicyDB().Exists("id",NewObject.managementPolicy)) {
            BadRequest(RESTAPI::Errors::UnknownManagementPolicyUUID);
            return;
        }

        NewObject.info.id = Daemon()->CreateUUID();
        NewObject.info.created = NewObject.info.modified = std::time(nullptr);
        NewObject.inUse.clear();

        if(DB_.CreateRecord(NewObject)) {

            Storage()->EntityDB().AddContact("id",NewObject.entity,DB_.Prefix(),NewObject.info.id);
            if(!NewObject.managementPolicy.empty())
                Storage()->PolicyDB().AddInUse("id",NewObject.managementPolicy,DB_.Prefix(),NewObject.info.id);

            ProvObjects::Contact    NewContact;
            Storage()->ContactDB().GetRecord("id", NewObject.info.id, NewContact);

            Poco::JSON::Object Answer;
            NewContact.to_json(Answer);
            ReturnObject(Answer);
            return;
        }
        InternalError(RESTAPI::Errors::RecordNotCreated);
    }

    void RESTAPI_contact_handler::DoPut() {
        std::string UUID = GetBinding(RESTAPI::Protocol::UUID,"");
        ProvObjects::Contact   Existing;
        if(UUID.empty() || !DB_.GetRecord("id", UUID, Existing)) {
            return NotFound();
        }

        auto RawObject = ParseStream();
        ProvObjects::Contact NewObject;
        if (!NewObject.from_json(RawObject)) {
            return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
        }

        for(auto &i:NewObject.info.notes) {
            i.createdBy = UserInfo_.userinfo.email;
            Existing.info.notes.insert(Existing.info.notes.begin(),i);
        }

        std::string MoveToPolicy, MoveFromPolicy;
        bool MovingPolicy=false;
        if(AssignIfPresent(RawObject,"managementPolicy",MoveToPolicy)) {
            if(!MoveToPolicy.empty() && !Storage()->PolicyDB().Exists("id",MoveToPolicy)) {
                return BadRequest(RESTAPI::Errors::UnknownManagementPolicyUUID);
            }
            MoveFromPolicy = Existing.managementPolicy;
            MovingPolicy = MoveToPolicy != Existing.managementPolicy;
        }

        std::string MoveToEntity,MoveFromEntity;
        bool        Movingentity=false;
        if(AssignIfPresent(RawObject,"entity",MoveToEntity) && MoveToEntity!=Existing.entity) {
            if(!MoveToEntity.empty() || !Storage()->EntityDB().Exists("id",MoveToEntity)) {
                return BadRequest(RESTAPI::Errors::EntityMustExist);
            }
            MoveFromEntity = Existing.entity;
            Movingentity = true ;
        }

        AssignIfPresent(RawObject,"name",Existing.info.name);
        AssignIfPresent(RawObject,"description",Existing.info.description);
        AssignIfPresent(RawObject, "title", Existing.title);
        AssignIfPresent(RawObject, "salutation", Existing.salutation);
        AssignIfPresent(RawObject, "firstname", Existing.firstname);
        AssignIfPresent(RawObject, "lastname", Existing.lastname);
        AssignIfPresent(RawObject, "initials", Existing.initials);
        AssignIfPresent(RawObject, "visual", Existing.visual);
        AssignIfPresent(RawObject, "primaryEmail", Existing.primaryEmail);
        AssignIfPresent(RawObject, "secondaryEmail", Existing.secondaryEmail);
        AssignIfPresent(RawObject, "accessPIN", Existing.accessPIN);
        if(RawObject->has("type"))
            Existing.type = NewObject.type;
        if(RawObject->has("mobiles"))
            Existing.mobiles = NewObject.mobiles;
        if(RawObject->has("phones"))
            Existing.phones = NewObject.phones;


        Existing.entity = MoveToEntity;
        Existing.info.modified = std::time(nullptr);
        Existing.managementPolicy = MoveToPolicy;

        if(DB_.UpdateRecord("id", UUID, Existing)) {

            if(MovingPolicy) {
                if(!MoveFromPolicy.empty())
                    Storage()->PolicyDB().DeleteInUse("id",MoveFromPolicy,DB_.Prefix(),Existing.info.id);
                if(!MoveToPolicy.empty())
                    Storage()->PolicyDB().AddInUse("id", MoveToPolicy, DB_.Prefix(), Existing.info.id);
            }

            if(Movingentity) {
                if(!MoveFromEntity.empty()) {
                    Storage()->EntityDB().DeleteContact("id", MoveFromEntity, DB_.Prefix(), Existing.info.id);
                }
                if(!MoveToEntity.empty()) {
                    Storage()->EntityDB().AddContact("id", MoveToEntity, DB_.Prefix(), Existing.info.id);
                }
            }

            ProvObjects::Contact    NewObjectAdded;
            DB_.GetRecord("id", UUID, NewObjectAdded);
            Poco::JSON::Object  Answer;
            NewObjectAdded.to_json(Answer);
            return ReturnObject(Answer);
        }
        InternalError(RESTAPI::Errors::RecordNotUpdated);
    }
}