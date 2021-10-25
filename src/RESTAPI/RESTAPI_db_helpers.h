//
// Created by stephane bourque on 2021-10-18.
//

#ifndef OWPROV_RESTAPI_DB_HELPERS_H
#define OWPROV_RESTAPI_DB_HELPERS_H

#include "RESTObjects/RESTAPI_ProvObjects.h"
#include "StorageService.h"
#include "framework/MicroService.h"

namespace OpenWifi {

    template <typename R, typename Q = decltype(R{}.entity)> void Extend_entity(const R &T, Poco::JSON::Object &EI ) {
        if constexpr(std::is_same_v<Q,std::string>) {
            if(!T.entity.empty()) {
                Poco::JSON::Object  EntObj;
                ProvObjects::Entity Entity;
                if(StorageService()->EntityDB().GetRecord("id",T.entity,Entity)) {
                    EntObj.set( "name", Entity.info.name);
                    EntObj.set( "description", Entity.info.description);
                }
                EI.set("entity",EntObj);
            }
        }
    }
    template <typename... Ts> void Extend_entity(Ts... args) {
        static_assert( sizeof...(args) == 2);
    }

    template <typename R, typename Q = decltype(R{}.managementPolicy)> void Extend_managementPolicy(const R &T, Poco::JSON::Object &EI ) {
        if constexpr(std::is_same_v<Q,std::string>) {
            if(!T.managementPolicy.empty()) {
                Poco::JSON::Object  PolObj;
                ProvObjects::ManagementPolicy Policy;
                if(StorageService()->PolicyDB().GetRecord("id",T.managementPolicy,Policy)) {
                    PolObj.set( "name", Policy.info.name);
                    PolObj.set( "description", Policy.info.description);
                }
                EI.set("managementPolicy",PolObj);
            }
        }
    }
    template <typename... Ts> void Extend_managementPolicy(Ts... args) {
        static_assert( sizeof...(args) == 2);
    }

    template <typename R, typename Q = decltype(R{}.venue)> void Extend_venue(const R &T, Poco::JSON::Object &EI ) {
        if constexpr(std::is_same_v<Q,std::string>) {
            if(!T.venue.empty()) {
                Poco::JSON::Object  EntObj;
                ProvObjects::Venue Venue;
                if(StorageService()->VenueDB().GetRecord("id",T.venue,Venue)) {
                    EntObj.set( "name", Venue.info.name);
                    EntObj.set( "description", Venue.info.description);
                }
                EI.set("venue",EntObj);
            }
        }
    }
    template <typename... Ts> void Extend_venue(Ts... args) {
        static_assert( sizeof...(args) == 2);
    }

    template <typename R, typename Q = decltype(R{}.contact)> void Extend_contact(const R &T, Poco::JSON::Object &EI ) {
        if constexpr(std::is_same_v<Q,std::string>) {
            if(!T.contact.empty()) {
                Poco::JSON::Object  EntObj;
                ProvObjects::Contact Contact;
                if(StorageService()->ContactDB().GetRecord("id",T.contact,Contact)) {
                    EntObj.set( "name", Contact.info.name);
                    EntObj.set( "description", Contact.info.description);
                }
                EI.set("contact",EntObj);
            }
        }
    }
    template <typename... Ts> void Extend_contact(Ts... args) {
        static_assert( sizeof...(args) == 2);
    }

    template <typename R, typename Q = decltype(R{}.location)> void Extend_location(const R &T, Poco::JSON::Object &EI ) {
        if constexpr(std::is_same_v<Q,std::string>) {
            if(!T.location.empty()) {
                Poco::JSON::Object  EntObj;
                ProvObjects::Location Location;
                if(StorageService()->LocationDB().GetRecord("id",T.location,Location)) {
                    EntObj.set( "name", Location.info.name);
                    EntObj.set( "description", Location.info.description);
                }
                EI.set("location",EntObj);
            }
        }
    }
    template <typename... Ts> void Extend_location(Ts... args) {
        static_assert( sizeof...(args) == 2);
    }

    template <typename R, typename Q = decltype(R{}.deviceConfiguration)> void Extend_deviceConfiguration(const R &T, Poco::JSON::Object &EI ) {
        if constexpr(std::is_same_v<Q,std::string>) {
            if(!T.deviceConfiguration.empty()) {
                Poco::JSON::Object  EntObj;
                ProvObjects::DeviceConfiguration DevConf;
                if(StorageService()->ConfigurationDB().GetRecord("id",T.deviceConfiguration,DevConf)) {
                    EntObj.set( "name", DevConf.info.name);
                    EntObj.set( "description", DevConf.info.description);
                }
                EI.set("deviceConfiguration",EntObj);
            }
        }
    }
    template <typename... Ts> void Extend_deviceConfiguration(Ts... args) {
        static_assert( sizeof...(args) == 2);
    }

    template <typename R> bool AddExtendedInfo(const R & T, Poco::JSON::Object &O) {
        Poco::JSON::Object  EI;
        Extend_entity(T,EI);
        Extend_deviceConfiguration(T,EI);
        Extend_location(T,EI);
        Extend_contact(T,EI);
        Extend_venue(T,EI);
        Extend_managementPolicy(T,EI);
        O.set("extendedInfo", EI);
        return true;
    }

    template <typename T> void MakeJSONObjectArray(const char * ArrayName, const std::vector<T> & V, RESTAPIHandler & R) {
        Poco::JSON::Array   ObjArray;
        for(const auto &i:V) {
            Poco::JSON::Object  Obj;
            i.to_json(Obj);
            if(R.NeedAdditionalInfo())
                AddExtendedInfo(i,Obj);
            ObjArray.add(Obj);
        }
        Poco::JSON::Object  Answer;
        Answer.set(ArrayName,ObjArray);
        return R.ReturnObject(Answer);
    }

    template <typename DB, typename Record> void ReturnRecordList(const char *ArrayName,DB & DBInstance, RESTAPIHandler & R) {
        auto UUIDs = Utils::Split(R.SelectedRecords());
        Poco::JSON::Array   ObjArr;
        for(const auto &i:UUIDs) {
            Record E;
            if(DBInstance.GetRecord("id",i,E)) {
                Poco::JSON::Object  Obj;
                E.to_json(Obj);
                if(R.NeedAdditionalInfo())
                    AddExtendedInfo(E,Obj);
                ObjArr.add(Obj);
            } else {
                return R.BadRequest("Unknown UUID:" + i);
            }
        }
        Poco::JSON::Object  Answer;
        Answer.set(ArrayName, ObjArr);
        return R.ReturnObject(Answer);
    }

}

#endif //OWPROV_RESTAPI_DB_HELPERS_H
