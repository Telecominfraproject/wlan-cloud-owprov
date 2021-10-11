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
            NotFound();
            return;
        }

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
            Poco::JSON::Object  Answer;
            Answer.set("entries", Inner);
            ReturnObject(Answer);
            return;
        }

        Poco::JSON::Object  Answer;
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
        std::string UUID = GetBinding(RESTAPI::Protocol::ID,"");
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

        if(NewObject.owner.empty() || !Storage()->EntityDB().Exists("id",NewObject.owner)) {
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
            Poco::JSON::Object Answer;
            NewObject.to_json(Answer);
            ReturnObject(Answer);
            return;
        }
        InternalError(RESTAPI::Errors::RecordNotCreated);
    }

    void RESTAPI_contact_handler::DoPut() {
        std::string UUID = GetBinding("uuid","");
        ProvObjects::Contact   Existing;
        if(UUID.empty() || !DB_.GetRecord("id", UUID, Existing)) {
            NotFound();
            return;
        }

        auto RawObject = ParseStream();
        ProvObjects::Contact NewObject;
        if (!NewObject.from_json(RawObject)) {
            BadRequest(RESTAPI::Errors::InvalidJSONDocument);
            return;
        }

        for(auto &i:NewObject.info.notes) {
            i.createdBy = UserInfo_.userinfo.email;
            Existing.info.notes.insert(Existing.info.notes.begin(),i);
        }

        std::string MoveEntity, MovePolicy;
        bool MovingEntity=false, MovingPolicy=false;
        if(AssignIfPresent(RawObject,"managementPolicy",MovePolicy)) {
            if(!MovePolicy.empty() && !Storage()->PolicyDB().Exists("id",MovePolicy)) {
                BadRequest(RESTAPI::Errors::UnknownManagementPolicyUUID);
                return;
            }
            MovingPolicy = MovePolicy != Existing.managementPolicy;
        }
        if(AssignIfPresent(RawObject,"entity",MoveEntity)) {
            if(MoveEntity.empty() || !Storage()->PolicyDB().Exists("id",MoveEntity)) {
                BadRequest(RESTAPI::Errors::EntityMustExist);
                return;
            }
            MovingEntity = MoveEntity != Existing.owner;
        }

        AssignIfPresent(RawObject,"name",Existing.info.name);
        AssignIfPresent(RawObject,"description",Existing.info.description);
        Existing.info.modified = std::time(nullptr);

        if(DB_.UpdateRecord("id", UUID, Existing)) {
            if(MovingPolicy) {
                if(!Existing.managementPolicy.empty())
                    Storage()->PolicyDB().DeleteInUse("id",Existing.managementPolicy,DB_.Prefix(),Existing.info.id);
                if(!MovePolicy.empty())
                    Storage()->PolicyDB().AddInUse("id", MovePolicy, DB_.Prefix(), Existing.info.id);
                Existing.managementPolicy = MovePolicy;
            }
            if(MovingEntity) {
                Existing.owner = MoveEntity;
            }
            DB_.UpdateRecord("id",UUID,Existing);

            ProvObjects::Contact    NewObjectAdded;
            DB_.GetRecord("id", UUID, NewObjectAdded);
            Poco::JSON::Object  Answer;
            NewObjectAdded.to_json(Answer);
            ReturnObject(Answer);
            return;
        }
        InternalError(RESTAPI::Errors::RecordNotUpdated);
    }
}