//
// Created by stephane bourque on 2021-10-18.
//

#include "RESTAPI_db_helpers.h"
#include "StorageService.h"

namespace OpenWifi {

    bool AddInventoryExtendedInfo(const ProvObjects::InventoryTag &T, Poco::JSON::Object &O) {
        Poco::JSON::Object  EI;
        if(!T.entity.empty()) {
            Poco::JSON::Object  EntObj;
            ProvObjects::Entity Entity;
            if(Storage()->EntityDB().GetRecord("id",T.entity,Entity)) {
                EntObj.set( "name", Entity.info.name);
                EntObj.set( "description", Entity.info.description);
            }
            EI.set("entity",EntObj);
        }
        if(!T.managementPolicy.empty()) {
            Poco::JSON::Object  PolObj;
            ProvObjects::ManagementPolicy Policy;
            if(Storage()->PolicyDB().GetRecord("id",T.managementPolicy,Policy)) {
                PolObj.set( "name", Policy.info.name);
                PolObj.set( "description", Policy.info.description);
            }
            EI.set("managementPolicy",PolObj);
        }
        if(!T.venue.empty()) {
            Poco::JSON::Object  EntObj;
            ProvObjects::Venue Venue;
            if(Storage()->VenueDB().GetRecord("id",T.venue,Venue)) {
                EntObj.set( "name", Venue.info.name);
                EntObj.set( "description", Venue.info.description);
            }
            EI.set("venue",EntObj);
        }
        if(!T.contact.empty()) {
            Poco::JSON::Object  EntObj;
            ProvObjects::Contact Contact;
            if(Storage()->ContactDB().GetRecord("id",T.contact,Contact)) {
                EntObj.set( "name", Contact.info.name);
                EntObj.set( "description", Contact.info.description);
            }
            EI.set("contact",EntObj);
        }
        if(!T.location.empty()) {
            Poco::JSON::Object  EntObj;
            ProvObjects::Location Location;
            if(Storage()->LocationDB().GetRecord("id",T.location,Location)) {
                EntObj.set( "name", Location.info.name);
                EntObj.set( "description", Location.info.description);
            }
            EI.set("location",EntObj);
        }
        if(!T.deviceConfiguration.empty()) {
            Poco::JSON::Object  EntObj;
            ProvObjects::DeviceConfiguration DevConf;
            if(Storage()->ConfigurationDB().GetRecord("id",T.deviceConfiguration,DevConf)) {
                EntObj.set( "name", DevConf.info.name);
                EntObj.set( "description", DevConf.info.description);
            }
            EI.set("deviceConfiguration",EntObj);
        }
        O.set("extendedInfo", EI);
        return true;
    }

}