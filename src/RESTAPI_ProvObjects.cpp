//
// Created by stephane bourque on 2021-08-04.
//

#include "RESTAPI_ProvObjects.h"
#include "RESTAPI_utils.h"

namespace OpenWifi::ProvObjects {

    void ObjectInfo::to_json(Poco::JSON::Object &Obj) const {
        RESTAPI_utils::field_to_json(Obj,"id",id);
        RESTAPI_utils::field_to_json(Obj,"name",name);
        RESTAPI_utils::field_to_json(Obj,"description",description);
        RESTAPI_utils::field_to_json(Obj,"created",created);
        RESTAPI_utils::field_to_json(Obj,"modified",modified);
        RESTAPI_utils::field_to_json(Obj,"notes",notes);
    }

    bool ObjectInfo::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            RESTAPI_utils::field_from_json(Obj,"id",id);
            RESTAPI_utils::field_from_json(Obj,"name",name);
            RESTAPI_utils::field_from_json(Obj,"description",description);
            RESTAPI_utils::field_from_json(Obj,"created",created);
            RESTAPI_utils::field_from_json(Obj,"modified",modified);
            RESTAPI_utils::field_from_json(Obj,"notes",notes);
            return true;
        } catch(...) {

        }
        return false;
    }

    void ManagementPolicyEntry::to_json(Poco::JSON::Object &Obj) const {
        RESTAPI_utils::field_to_json( Obj,"users",users);
        RESTAPI_utils::field_to_json( Obj,"resources",resources);
        RESTAPI_utils::field_to_json( Obj,"access",access);
        RESTAPI_utils::field_to_json( Obj,"policy",policy);
    }

    bool ManagementPolicyEntry::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            RESTAPI_utils::field_from_json( Obj,"users",users);
            RESTAPI_utils::field_from_json( Obj,"resources",resources);
            RESTAPI_utils::field_from_json( Obj,"access",access);
            RESTAPI_utils::field_from_json( Obj,"policy",policy);
            return true;
        } catch(...) {

        }
        return false;
    }

    void ManagementPolicy::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        RESTAPI_utils::field_to_json(Obj, "entries", entries);
        RESTAPI_utils::field_to_json(Obj, "inUse", inUse);
    }

    bool ManagementPolicy::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            RESTAPI_utils::field_from_json(Obj, "entries", entries);
            RESTAPI_utils::field_from_json(Obj, "inUse", inUse);
            return true;
        } catch(...) {

        }
        return false;
    }

    void Entity::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        RESTAPI_utils::field_to_json( Obj,"parent",parent);
        RESTAPI_utils::field_to_json( Obj,"venues",venues);
        RESTAPI_utils::field_to_json( Obj,"children",children);
        RESTAPI_utils::field_to_json( Obj,"contacts",contacts);
        RESTAPI_utils::field_to_json( Obj,"locations",locations);
        RESTAPI_utils::field_to_json( Obj,"managementPolicy",managementPolicy);
    }

    bool Entity::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            RESTAPI_utils::field_from_json( Obj,"parent",parent);
            RESTAPI_utils::field_from_json( Obj,"venues",venues);
            RESTAPI_utils::field_from_json( Obj,"children",children);
            RESTAPI_utils::field_from_json( Obj,"contacts",contacts);
            RESTAPI_utils::field_from_json( Obj,"locations",locations);
            RESTAPI_utils::field_from_json( Obj,"managementPolicy",managementPolicy);
            return true;
        } catch(...) {

        }
        return false;
    }

    void DiGraphEntry::to_json(Poco::JSON::Object &Obj) const {
        RESTAPI_utils::field_to_json( Obj,"parent",parent);
        RESTAPI_utils::field_to_json( Obj,"child",child);
    }

    bool DiGraphEntry::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            RESTAPI_utils::field_from_json( Obj,"parent",parent);
            RESTAPI_utils::field_from_json( Obj,"child",child);
            return true;
        } catch (...) {

        }
        return false;
    }

    void Venue::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        RESTAPI_utils::field_to_json( Obj,"parent",parent);
        RESTAPI_utils::field_to_json( Obj,"owner",entity);
        RESTAPI_utils::field_to_json( Obj,"children",children);
        RESTAPI_utils::field_to_json( Obj,"devices",devices);
        RESTAPI_utils::field_to_json( Obj,"topology",topology);
        RESTAPI_utils::field_to_json( Obj,"parent",parent);
        RESTAPI_utils::field_to_json( Obj,"design",design);
        RESTAPI_utils::field_to_json( Obj,"managementPolicy",managementPolicy);
    }

    bool Venue::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            RESTAPI_utils::field_from_json( Obj,"parent",parent);
            RESTAPI_utils::field_from_json( Obj,"owner",entity);
            RESTAPI_utils::field_from_json( Obj,"children",children);
            RESTAPI_utils::field_from_json( Obj,"devices",devices);
            RESTAPI_utils::field_from_json( Obj,"topology",topology);
            RESTAPI_utils::field_from_json( Obj,"parent",parent);
            RESTAPI_utils::field_from_json( Obj,"design",design);
            RESTAPI_utils::field_from_json( Obj,"managementPolicy",managementPolicy);
            return true;
        } catch (...) {

        }
        return false;
    }

    void UserInfoDigest::to_json(Poco::JSON::Object &Obj) const {
        RESTAPI_utils::field_to_json( Obj,"id",id);
        RESTAPI_utils::field_to_json( Obj,"owner",loginId);
        RESTAPI_utils::field_to_json( Obj,"children",userType);
    }

    bool UserInfoDigest::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            RESTAPI_utils::field_from_json( Obj,"id",id);
            RESTAPI_utils::field_from_json( Obj,"owner",loginId);
            RESTAPI_utils::field_from_json( Obj,"children",userType);
            return true;
        } catch(...) {
        }
        return false;
    }

    void ManagementRole::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        RESTAPI_utils::field_to_json( Obj,"managementPolicy",managementPolicy);
        RESTAPI_utils::field_to_json( Obj,"users",users);
    }

    bool ManagementRole::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            RESTAPI_utils::field_from_json( Obj,"managementPolicy",managementPolicy);
            RESTAPI_utils::field_from_json( Obj,"users",users);
            return true;
        } catch(...) {
        }
        return false;
    }

    void Location::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        RESTAPI_utils::field_to_json( Obj,"type",OpenWifi::ProvObjects::to_string(type));
        RESTAPI_utils::field_to_json( Obj,"buildingName",buildingName);
        RESTAPI_utils::field_to_json( Obj,"addressLines",addressLines);
        RESTAPI_utils::field_to_json( Obj,"city",city);
        RESTAPI_utils::field_to_json( Obj,"state",state);
        RESTAPI_utils::field_to_json( Obj,"postal",postal);
        RESTAPI_utils::field_to_json( Obj,"country",country);
        RESTAPI_utils::field_to_json( Obj,"phones",phones);
        RESTAPI_utils::field_to_json( Obj,"mobiles",mobiles);
        RESTAPI_utils::field_to_json( Obj,"geoCode",geoCode);
        RESTAPI_utils::field_to_json( Obj,"inUse",inUse);
    }

    bool Location::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            std::string tmp_type;
            RESTAPI_utils::field_from_json( Obj,"type", tmp_type);
            type = location_from_string(tmp_type);
            RESTAPI_utils::field_from_json( Obj,"buildingName",buildingName);
            RESTAPI_utils::field_from_json( Obj,"addressLines",addressLines);
            RESTAPI_utils::field_from_json( Obj,"city",city);
            RESTAPI_utils::field_from_json( Obj,"state",state);
            RESTAPI_utils::field_from_json( Obj,"postal",postal);
            RESTAPI_utils::field_from_json( Obj,"country",country);
            RESTAPI_utils::field_from_json( Obj,"phones",phones);
            RESTAPI_utils::field_from_json( Obj,"mobiles",mobiles);
            RESTAPI_utils::field_from_json( Obj,"geoCode",geoCode);
            RESTAPI_utils::field_from_json( Obj,"inUse",inUse);
            return true;
        } catch (...) {

        }
        return false;
    }

    void Contact::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        RESTAPI_utils::field_to_json( Obj,"type", to_string(type));
        RESTAPI_utils::field_to_json( Obj,"title",title);
        RESTAPI_utils::field_to_json( Obj,"salutation",salutation);
        RESTAPI_utils::field_to_json( Obj,"firstname",firstname);
        RESTAPI_utils::field_to_json( Obj,"lastname",lastname);
        RESTAPI_utils::field_to_json( Obj,"initials",initials);
        RESTAPI_utils::field_to_json( Obj,"visual",visual);
        RESTAPI_utils::field_to_json( Obj,"mobiles",mobiles);
        RESTAPI_utils::field_to_json( Obj,"phones",phones);
        RESTAPI_utils::field_to_json( Obj,"primaryEmail",primaryEmail);
        RESTAPI_utils::field_to_json( Obj,"secondaryEmail",secondaryEmail);
        RESTAPI_utils::field_to_json( Obj,"accessPIN",accessPIN);
        RESTAPI_utils::field_to_json( Obj,"inUse",inUse);
    }

    bool Contact::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            std::string tmp_type;
            RESTAPI_utils::field_from_json( Obj,"type", tmp_type);
            type = contact_from_string(tmp_type);
            RESTAPI_utils::field_from_json( Obj,"title",title);
            RESTAPI_utils::field_from_json( Obj,"salutation",salutation);
            RESTAPI_utils::field_from_json( Obj,"firstname",firstname);
            RESTAPI_utils::field_from_json( Obj,"lastname",lastname);
            RESTAPI_utils::field_from_json( Obj,"initials",initials);
            RESTAPI_utils::field_from_json( Obj,"visual",visual);
            RESTAPI_utils::field_from_json( Obj,"mobiles",mobiles);
            RESTAPI_utils::field_from_json( Obj,"phones",phones);
            RESTAPI_utils::field_from_json( Obj,"primaryEmail",primaryEmail);
            RESTAPI_utils::field_from_json( Obj,"secondaryEmail",secondaryEmail);
            RESTAPI_utils::field_from_json( Obj,"accessPIN",accessPIN);
            RESTAPI_utils::field_from_json( Obj,"inUse",inUse);
            return true;
        } catch (...) {

        }
        return false;
    }

    void ServiceConfiguration::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        managementPolicy.to_json(Obj);
    }

    bool ServiceConfiguration::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            managementPolicy.from_json(Obj);
            return true;
        } catch(...) {

        }
        return false;
    }

    void InventoryTag::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        RESTAPI_utils::field_to_json(Obj, "serialNumber", serialNumber);
        RESTAPI_utils::field_to_json(Obj, "venue", venue);
        RESTAPI_utils::field_to_json(Obj, "entity", entity);
        RESTAPI_utils::field_to_json(Obj, "subscriber", subscriber);
        RESTAPI_utils::field_to_json(Obj, "deviceType", deviceType);
        RESTAPI_utils::field_to_json(Obj, "qrCode", qrCode);
        RESTAPI_utils::field_to_json(Obj, "geoCode", geoCode);
        RESTAPI_utils::field_to_json(Obj, "location", location);
        RESTAPI_utils::field_to_json(Obj, "contact", contact);
    }

    bool InventoryTag::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            RESTAPI_utils::field_from_json( Obj,"serialNumber",serialNumber);
            RESTAPI_utils::field_from_json( Obj,"venue",venue);
            RESTAPI_utils::field_from_json( Obj,"entity",entity);
            RESTAPI_utils::field_from_json( Obj,"subscriber",subscriber);
            RESTAPI_utils::field_from_json( Obj,"deviceType",deviceType);
            RESTAPI_utils::field_from_json(Obj, "qrCode", qrCode);
            RESTAPI_utils::field_from_json( Obj,"geoCode",geoCode);
            RESTAPI_utils::field_from_json( Obj,"location",location);
            RESTAPI_utils::field_from_json( Obj,"contact",contact);
            return true;
        } catch(...) {

        }
        return false;
    }

    void DeviceConfiguration::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        managementPolicy.to_json(Obj);
        RESTAPI_utils::field_to_json( Obj,"deviceTypes",deviceTypes);
        RESTAPI_utils::field_to_json( Obj,"configuration",configuration);
    }

    bool DeviceConfiguration::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            managementPolicy.from_json(Obj);
            RESTAPI_utils::field_from_json( Obj,"deviceTypes",deviceTypes);
            RESTAPI_utils::field_from_json( Obj,"configuration",configuration);
            return true;
        } catch(...) {

        }
        return false;
    }

    void Report::to_json(Poco::JSON::Object &Obj) const {
        RESTAPI_utils::field_to_json(Obj, "snapshot", snapShot);
        RESTAPI_utils::field_to_json(Obj, "devices", tenants);
    };

    void Report::reset() {
        tenants.clear();
    }

};
