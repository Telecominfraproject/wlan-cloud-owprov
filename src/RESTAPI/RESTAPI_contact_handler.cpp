//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "RESTAPI_contact_handler.h"

#include "framework/ow_constants.h"
#include "RESTObjects/RESTAPI_ProvObjects.h"
#include "RESTAPI_db_helpers.h"

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
            if(StorageService()->ExpandInUse(Existing.inUse,M,Errors)) {
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
        } else if(QB_.AdditionalInfo) {
            AddExtendedInfo(Existing, Answer);
        }

        Existing.to_json(Answer);
        ReturnObject(Answer);
    }

    void RESTAPI_contact_handler::DoDelete() {

        std::string UUID = GetBinding("uuid","");
        ProvObjects::Contact   Existing;
        if(UUID.empty() || !DB_.GetRecord("id", UUID, Existing)) {
            return NotFound();
        }

        bool Force=false;
        std::string Arg;
        if(HasParameter("force",Arg) && Arg=="true")
            Force=true;

        if(!Force && !Existing.inUse.empty()) {
            return BadRequest(RESTAPI::Errors::StillInUse);
        }

        if(DB_.DeleteRecord("id",UUID)) {
            if(!Existing.entity.empty())
                StorageService()->EntityDB().DeleteLocation("id",Existing.entity,UUID);
            return OK();
        }
        InternalError(RESTAPI::Errors::CouldNotBeDeleted);
    }

    void RESTAPI_contact_handler::DoPost() {
        std::string UUID = GetBinding(RESTAPI::Protocol::UUID,"");

        if(UUID.empty()) {
            return BadRequest(RESTAPI::Errors::MissingUUID);
        }

        auto Obj = ParseStream();
        ProvObjects::Contact NewObject;
        if (!NewObject.from_json(Obj)) {
            return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
        }

        if(!ProvObjects::CreateObjectInfo(Obj,UserInfo_.userinfo,NewObject.info)) {
            return BadRequest(RESTAPI::Errors::NameMustBeSet);
        }

        if(NewObject.entity.empty() || !StorageService()->EntityDB().Exists("id",NewObject.entity)) {
            return BadRequest(RESTAPI::Errors::EntityMustExist);
        }

        if(!NewObject.managementPolicy.empty() && !StorageService()->PolicyDB().Exists("id",NewObject.managementPolicy)) {
            return BadRequest(RESTAPI::Errors::UnknownManagementPolicyUUID);
        }

        NewObject.inUse.clear();

        if(DB_.CreateRecord(NewObject)) {

            StorageService()->EntityDB().AddContact("id",NewObject.entity,NewObject.info.id);
            if(!NewObject.managementPolicy.empty())
                StorageService()->PolicyDB().AddInUse("id",NewObject.managementPolicy,DB_.Prefix(),NewObject.info.id);

            ProvObjects::Contact    NewContact;
            StorageService()->ContactDB().GetRecord("id", NewObject.info.id, NewContact);

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

        if(!UpdateObjectInfo(RawObject, UserInfo_.userinfo, Existing.info)) {
            return BadRequest(RESTAPI::Errors::NameMustBeSet);
        }

        std::string MoveToPolicy, MoveFromPolicy;
        if(AssignIfPresent(RawObject,"managementPolicy",MoveToPolicy)) {
            if(!MoveToPolicy.empty() && !StorageService()->PolicyDB().Exists("id",MoveToPolicy)) {
                return BadRequest(RESTAPI::Errors::UnknownManagementPolicyUUID);
            }
            MoveFromPolicy = Existing.managementPolicy;
            Existing.managementPolicy = MoveToPolicy;
        }

        std::string MoveToEntity,MoveFromEntity;
        if(AssignIfPresent(RawObject,"entity",MoveToEntity)) {
            if(!MoveToEntity.empty() && !StorageService()->EntityDB().Exists("id",MoveToEntity)) {
                return BadRequest(RESTAPI::Errors::EntityMustExist);
            }
            MoveFromEntity = Existing.entity;
            Existing.entity = MoveToEntity;
        }

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

        if(DB_.UpdateRecord("id", UUID, Existing)) {

            MoveUsage(StorageService()->PolicyDB(),DB_,MoveFromPolicy,MoveToPolicy,Existing.info.id);
            ManageMembership(StorageService()->EntityDB(),&ProvObjects::Entity::contacts,MoveFromEntity,MoveToEntity,Existing.info.id);

            ProvObjects::Contact    NewObjectAdded;
            DB_.GetRecord("id", UUID, NewObjectAdded);
            Poco::JSON::Object  Answer;
            NewObjectAdded.to_json(Answer);
            return ReturnObject(Answer);
        }
        InternalError(RESTAPI::Errors::RecordNotUpdated);
    }
}