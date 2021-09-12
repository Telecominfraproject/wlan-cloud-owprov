//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//


#include "RESTAPI_entity_handler.h"
#include "RESTAPI_ProvObjects.h"
#include "StorageService.h"

#include "Poco/JSON/Parser.h"
#include "Daemon.h"
#include "RESTAPI_SecurityObjects.h"
#include "RESTAPI_utils.h"
#include "RESTAPI_errors.h"

namespace OpenWifi{

    void RESTAPI_entity_handler::DoGet() {
        std::string UUID = GetBinding("uuid", "");
        if(UUID.empty()) {
            BadRequest(RESTAPI::Errors::MissingUUID);
            return;
        }

        ProvObjects::Entity E;
        if(Storage()->EntityDB().GetRecord("id",UUID,E)) {
            Poco::JSON::Object Answer;
            E.to_json(Answer);
            ReturnObject(Answer);
            return;
        }
        NotFound();
    }

    void RESTAPI_entity_handler::DoDelete() {

        std::string UUID = GetBinding("uuid", "");
        if(UUID.empty()) {
            BadRequest(RESTAPI::Errors::MissingUUID);
            return;
        }

        if(UUID == EntityDB::RootUUID()) {
            BadRequest(RESTAPI::Errors::CannotDeleteRoot);
            return;
        }

        ProvObjects::Entity E;
        if(!Storage()->EntityDB().GetRecord("id",UUID,E)) {
            NotFound();
            return;
        }

        if(!E.children.empty()) {
            BadRequest(RESTAPI::Errors::StillInUse);
            return;
        }

        if(Storage()->EntityDB().DeleteRecord("id",UUID)) {
            Storage()->EntityDB().DeleteChild("id",E.parent,UUID);
            OK();
            return;
        }
        NotFound();
    }

    void RESTAPI_entity_handler::DoPost() {
        std::string UUID = GetBinding("uuid", "");
        if(UUID.empty()) {
            BadRequest(RESTAPI::Errors::MissingUUID);
            return;
        }

        if(!Storage()->EntityDB().RootExists() && UUID != EntityDB::RootUUID()) {
            BadRequest(RESTAPI::Errors::MustCreateRootFirst);
            return;
        }

        auto Obj = ParseStream();
        ProvObjects::Entity NewEntity;
        if (!NewEntity.from_json(Obj)) {
            BadRequest(RESTAPI::Errors::InvalidJSONDocument);
            return;
        }
        NewEntity.info.modified = NewEntity.info.created = std::time(nullptr);

        for(auto &i:NewEntity.info.notes)
            i.createdBy = UserInfo_.userinfo.email;

        //  When creating an entity, it cannot have any relations other that parent, notes, name, description. Everything else
        //  must be conveyed through PUT.
        NewEntity.info.id = (UUID==EntityDB::RootUUID()) ? UUID : Daemon()->CreateUUID() ;
        if(UUID==EntityDB::RootUUID()) {
            NewEntity.parent="";
        } else if(NewEntity.parent.empty() || !Storage()->EntityDB().Exists("id",NewEntity.parent)) {
            BadRequest(RESTAPI::Errors::ParentUUIDMustExist);
            return;
        }

        if(!NewEntity.deviceConfiguration.empty() && !Storage()->ConfigurationDB().Exists("id",NewEntity.deviceConfiguration)) {
            BadRequest(RESTAPI::Errors::ConfigurationMustExist);
            return;
        }

        NewEntity.venues.clear();
        NewEntity.children.clear();
        NewEntity.contacts.clear();
        NewEntity.locations.clear();

        if(Storage()->EntityDB().CreateShortCut(NewEntity)) {
            if(UUID==EntityDB::RootUUID())
                Storage()->EntityDB().CheckForRoot();
            else {
                Storage()->EntityDB().AddChild("id",NewEntity.parent,NewEntity.info.id);
            }

            if(!NewEntity.deviceConfiguration.empty())
                Storage()->ConfigurationDB().AddInUse("id",NewEntity.deviceConfiguration,Storage()->EntityDB().Prefix(),NewEntity.info.id);

            Poco::JSON::Object  Answer;
            NewEntity.to_json(Answer);
            ReturnObject(Answer);
            return;
        }
        NotFound();
    }

    /*
     * Put is a complex operation, it contains commands only.
     *      addContact=UUID, delContact=UUID,
     *      addLocation=UUID, delLocation=UUID,
     *      addVenue=UUID, delVenue=UUID,
     *      addEntity=UUID, delEntity=UUID
     *      addDevice=UUID, delDevice=UUID
     */

    void RESTAPI_entity_handler::DoPut() {
        std::string UUID = GetBinding("uuid", "");
        if(UUID.empty()) {
            BadRequest(RESTAPI::Errors::MissingUUID);
            return;
        }

        ProvObjects::Entity Existing;
        if(!Storage()->EntityDB().GetRecord("id",UUID,Existing)) {
            NotFound();
            return;
        }

        auto RawObject = ParseStream();
        ProvObjects::Entity NewEntity;
        if(!NewEntity.from_json(RawObject)) {
            BadRequest(RESTAPI::Errors::InvalidJSONDocument);
            return;
        }

        std::string OldConfiguration;
        if(!NewEntity.deviceConfiguration.empty() && !Storage()->ConfigurationDB().Exists("id",NewEntity.deviceConfiguration)) {
            BadRequest(RESTAPI::Errors::ConfigurationMustExist);
            return;
        } else {
            OldConfiguration = Existing.deviceConfiguration;
            Existing.deviceConfiguration = NewEntity.deviceConfiguration;
        }

        for(auto &i:NewEntity.info.notes) {
            i.createdBy = UserInfo_.userinfo.email;
            Existing.info.notes.insert(Existing.info.notes.begin(),i);
        }

        if(!NewEntity.info.name.empty())
            Existing.info.name = NewEntity.info.name;
        if(!NewEntity.info.description.empty())
            Existing.info.description = NewEntity.info.description;

        Existing.info.modified = std::time(nullptr);

        std::string Error;
        if(!Storage()->Validate(Parameters_,Error)) {
            BadRequest(Error);
            return;
        }

        if(Storage()->EntityDB().UpdateRecord("id",UUID,Existing)) {
            for(const auto &i:*Request) {
                std::string Child{i.second};
                auto UUID_parts = Utils::Split(Child,':');
                if(i.first=="add" && UUID_parts[0] == "con") {
                    Storage()->EntityDB().AddContact("id", UUID, UUID_parts[1]);
                    Storage()->ContactDB().AddInUse("id",UUID_parts[1],Storage()->EntityDB().Prefix(), UUID);
                } else if (i.first == "del" && UUID_parts[0] == "con") {
                    Storage()->EntityDB().DeleteContact("id", UUID, UUID_parts[1]);
                    Storage()->ContactDB().DeleteInUse("id",UUID_parts[1],Storage()->EntityDB().Prefix(),UUID);
                } else if (i.first == "add" && UUID_parts[0] == "loc") {
                    Storage()->EntityDB().AddLocation("id", UUID, UUID_parts[1]);
                    Storage()->LocationDB().AddInUse("id",UUID_parts[1],Storage()->EntityDB().Prefix(),UUID);
                } else if (i.first == "del" && UUID_parts[0] == "loc") {
                    Storage()->EntityDB().DeleteLocation("id", UUID, UUID_parts[1]);
                    Storage()->LocationDB().DeleteInUse("id",UUID_parts[1],Storage()->EntityDB().Prefix(),UUID);
                }
            }

            if(!OldConfiguration.empty()) {
                Storage()->ConfigurationDB().DeleteInUse("id", OldConfiguration, Storage()->EntityDB().Prefix(), Existing.info.id);
            }

            if(!NewEntity.deviceConfiguration.empty()) {
                Storage()->ConfigurationDB().AddInUse("id", NewEntity.deviceConfiguration, Storage()->EntityDB().Prefix(), Existing.info.id);
            }

            Poco::JSON::Object  Answer;
            ProvObjects::Entity NewRecord;
            Storage()->EntityDB().GetRecord("id",UUID, NewRecord);
            NewRecord.to_json(Answer);
            ReturnObject(Answer);
            return;
        } else {
            BadRequest(RESTAPI::Errors::RecordNotUpdated);
        }
    }
}