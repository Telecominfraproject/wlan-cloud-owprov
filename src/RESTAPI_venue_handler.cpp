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

namespace OpenWifi{

    void RESTAPI_venue_handler::DoGet() {

        try {
            std::string UUID = GetBinding("uuid", "");

            if(UUID.empty()) {
                BadRequest("Missing UUID");
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
            return;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest("Internal error.");
    }

    void RESTAPI_venue_handler::DoDelete() {
        try {
            std::string UUID = GetBinding("uuid", "");

            if(UUID.empty()) {
                BadRequest("Missing UUID");
                return;
            }

            ProvObjects::Venue V;
            if(Storage()->VenueDB().GetRecord("id",UUID,V)) {

                if(!V.children.empty() || !V.devices.empty()) {
                    BadRequest("Venue still has children or devices.");
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
            return;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest("Internal error.");
    }

    void RESTAPI_venue_handler::DoPost() {
        try {
            auto Obj = ParseStream();
            ProvObjects::Venue V;
            if (!V.from_json(Obj)) {
                BadRequest("Post document is malformed JSON.");
                return;
            }

            if(V.parent.empty() && V.entity.empty()) {
                BadRequest("Parent or Entity must be set.");
                return;
            }

            if(!V.parent.empty() && !V.entity.empty()) {
                BadRequest("Only one of Parent or Entity must be set.");
                return;
            }

            if(!V.parent.empty() && !Storage()->VenueDB().Exists("id",V.parent)) {
                BadRequest("Parent does not exist.");
                return;
            }

            if(V.entity == EntityDB::RootUUID()) {
                BadRequest("Entity cannot be root.");
                return;
            }

            if(!V.entity.empty() && !Storage()->EntityDB().Exists("id",V.entity)) {
                BadRequest("Entity does not exist.");
                return;
            }

            if(!V.contact.empty() && !Storage()->ContactDB().Exists("id",V.contact)) {
                BadRequest("Contact does not exist.");
                return;
            }

            if(!V.location.empty() && !Storage()->LocationDB().Exists("id",V.location)) {
                BadRequest("Location does not exist.");
                return;
            }

            if(!V.managementPolicy.empty() && !Storage()->PolicyDB().Exists("id",V.managementPolicy)) {
                BadRequest("ManagementPolicy does not exist.");
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
            return;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest("Internal error.");
    }

    void RESTAPI_venue_handler::DoPut() {
        try {
            std::string UUID = GetBinding("uuid","");

            if(UUID.empty()) {
                BadRequest("Missing UUID.");
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
                BadRequest("Entity " + MoveEntity + " does not exist.");
                return;
            }

            std::string MoveVenue;
            if(AssignIfPresent(RawObject, "venue", MoveVenue) && !Storage()->VenueDB().Exists("id",MoveVenue)) {
                BadRequest("Venue " + MoveVenue + " does not exist.");
                return;
            }

            std::string MoveLocation;
            if(AssignIfPresent(RawObject,"location",MoveLocation) && !Storage()->LocationDB().Exists("id",MoveLocation)) {
                BadRequest("Location " + MoveLocation + " does not exist.");
                return;
            }

            std::string MoveContact;
            if(AssignIfPresent(RawObject,"contact",MoveContact) && !Storage()->ContactDB().Exists("id",MoveContact)) {
                BadRequest("Contact " + MoveContact + " does not exist.");
                return;
            }

            std::string MovePolicy;
            if(AssignIfPresent(RawObject,"managementPolicy",MoveContact) && !Storage()->PolicyDB().Exists("id",MovePolicy)) {
                BadRequest("Policy " + MovePolicy + " does not exist.");
                return;
            }

            if(Storage()->VenueDB().UpdateRecord("id", UUID, ExistingVenue)) {

                // todo
                ProvObjects::Venue AddedRecord;
                Storage()->VenueDB().GetRecord("id",UUID,AddedRecord);
                Poco::JSON::Object  Answer;
                AddedRecord.to_json(Answer);
                ReturnObject(Answer);
                return;
            }
            NotFound();
            return;
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest("Internal error.");
    }
}