//
// Created by stephane bourque on 2021-08-04.
//

#ifndef OWPROV_RESTAPI_PROVOBJECTS_H
#define OWPROV_RESTAPI_PROVOBJECTS_H

#include <string>
#include "RESTAPI_SecurityObjects.h"

namespace OpenWifi::ProvObjects {

    struct ObjectInfo {
        uCentral::Types::UUID_t   id;
        std::string     name;
        std::string     description;
        uCentral::SecurityObjects::NoteInfoVec notes;
        uint64_t        created;
        uint64_t        modified;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct ManagementPolicyEntry {
        uCentral::Types::UUIDvec_t users;
        uCentral::Types::UUIDvec_t resources;
        uCentral::Types::StringVec access;
        std::string policy;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct ManagementPolicy {
        ObjectInfo  info;
        std::vector<ManagementPolicyEntry>  entries;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct Entity {
        ObjectInfo  info;
        uCentral::Types::UUID_t parent;
        uCentral::Types::UUIDvec_t children;
        uCentral::Types::UUIDvec_t venues;
        uCentral::Types::UUIDvec_t managers;
        uCentral::Types::UUIDvec_t contacts;
        uCentral::Types::UUIDvec_t locations;
        uCentral::Types::UUID_t managementPolicy;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct DiGraphEntry {
        uCentral::Types::UUID_t parent;
        uCentral::Types::UUID_t child;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    typedef std::vector<DiGraphEntry>   DiGraph;

    struct Venue {
        ObjectInfo  info;
        uCentral::Types::UUID_t owner;
        uCentral::Types::UUID_t parent;
        uCentral::Types::UUIDvec_t children;
        uCentral::Types::UUIDvec_t managers;
        uCentral::Types::UUID_t managementPolicy;
        uCentral::Types::UUIDvec_t devices;
        DiGraph topology;
        std::string design;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct ManagementGroup {
        ObjectInfo  info;
        ManagementPolicy managementPolicy;
        uCentral::Types::UUIDvec_t managers;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    enum LocationType {
        LT_SERVICE, LT_EQUIPMENT, LT_AUTO, LT_MANUAL,
        LT_SPECIAL, LT_UNKNOWN, LT_CORPORATE
    };

    inline std::string to_string(LocationType L) {
        switch(L) {
            case LT_SERVICE: return "SERVICE";
            case LT_EQUIPMENT: return "EQUIPMENT";
            case LT_AUTO: return "AUTO";
            case LT_MANUAL: return "MANUAL";
            case LT_SPECIAL: return "SPECIAL";
            case LT_UNKNOWN: return "UNKNOWN";
            case LT_CORPORATE: return "CORPORATE";
            default: return "UNKNOWN";
        }
    }

    inline LocationType location_from_string(const std::string &S) {
        if(!Poco::icompare(S,"SERVICE"))
            return LT_SERVICE;
        else if(!Poco::icompare(S,"EQUIPMENT"))
            return LT_EQUIPMENT;
        else if(!Poco::icompare(S,"AUTO"))
            return LT_AUTO;
        else if(!Poco::icompare(S,"MANUAL"))
            return LT_MANUAL;
        else if(!Poco::icompare(S,"SPECIAL"))
            return LT_SPECIAL;
        else if(!Poco::icompare(S,"UNKNOWN"))
            return LT_UNKNOWN;
        else if(!Poco::icompare(S,"CORPORATE"))
            return LT_CORPORATE;
        return LT_UNKNOWN;
    }

    struct LocationTypeS {
        LocationType    type;
        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct Location {
        ObjectInfo  info;
        LocationType type;
        std::string buildingName;
        uCentral::Types::StringVec addressLines;
        std::string city;
        std::string state;
        std::string postal;
        std::string country;
        uCentral::Types::StringVec phones;
        uCentral::Types::UUID_t contact;
        uCentral::Types::StringVec mobiles;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    enum ContactType {
        CT_SUBSCRIBER, CT_USER, CT_INSTALLER, CT_CSR, CT_MANAGER,
        CT_BUSINESSOWNER, CT_TECHNICIAN, CT_CORPORATE, CT_UNKNOWN
    };

    inline std::string to_string(ContactType L) {
        switch(L) {
            case CT_SUBSCRIBER: return "SUBSCRIBER";
            case CT_USER: return "USER";
            case CT_INSTALLER: return "INSTALLER";
            case CT_CSR: return "CSR";
            case CT_MANAGER: return "MANAGER";
            case CT_BUSINESSOWNER: return "BUSINESSOWNER";
            case CT_TECHNICIAN: return "TECHNICIAN";
            case CT_CORPORATE: return "CORPORATE";
            case CT_UNKNOWN: return "UNKNOWN";
            default: return "UNKNOWN";
        }
    }

    inline ContactType contact_from_string(const std::string &S) {
        if(!Poco::icompare(S,"SUBSCRIBER"))
            return CT_SUBSCRIBER;
        else if(!Poco::icompare(S,"USER"))
            return CT_USER;
        else if(!Poco::icompare(S,"INSTALLER"))
            return CT_INSTALLER;
        else if(!Poco::icompare(S,"CSR"))
            return CT_CSR;
        else if(!Poco::icompare(S,"BUSINESSOWNER"))
            return CT_BUSINESSOWNER;
        else if(!Poco::icompare(S,"TECHNICIAN"))
            return CT_TECHNICIAN;
        else if(!Poco::icompare(S,"CORPORATE"))
            return CT_CORPORATE;
        else if(!Poco::icompare(S,"UNKNOWN"))
            return CT_UNKNOWN;
        return CT_UNKNOWN;
    }

    struct Contact {
        ObjectInfo  info;
        ContactType type;
        std::string title;
        std::string salutation;
        std::string firstname;
        std::string lastname;
        std::string initials;
        std::string visual;
        uCentral::Types::StringVec mobiles;
        uCentral::Types::StringVec phones;
        uCentral::Types::UUID_t location;
        std::string primaryEmail;
        std::string secondaryEmail;
        std::string accessPIN;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct ServiceConfiguration {
        ObjectInfo info;
        ManagementPolicy managementPolicy;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct DeviceConfiguration {
        ObjectInfo info;
        ManagementPolicy managementPolicy;
        uCentral::Types::StringVec deviceTypes;
        std::string configuration;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct InventoryTag {
        ObjectInfo      info;
        std::string     serialNumber;
        std::string     venue;
        std::string     entity;
        std::string     subEntity;
        std::string     subVenue;
        std::string     subscriber;
        std::string     deviceType;
        std::string     qrCode;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct Report {
        uint64_t            snapShot;
        uCentral::Types::CountedMap   tenants;

        void        reset();
        void to_json(Poco::JSON::Object &Obj) const;
    };
};


#endif //OWPROV_RESTAPI_PROVOBJECTS_H
