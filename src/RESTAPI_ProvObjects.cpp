//
// Created by stephane bourque on 2021-08-04.
//

#include "RESTAPI_ProvObjects.h"
#include "RESTAPI_utils.h"

using uCentral::RESTAPI_utils::field_from_json;
using uCentral::RESTAPI_utils::field_to_json;

namespace OpenWifi::ProvObjects {

    void ObjectInfo::to_json(Poco::JSON::Object &Obj) const {
        uCentral::RESTAPI_utils::field_to_json(Obj,"id",id);
        uCentral::RESTAPI_utils::field_to_json(Obj,"name",name);
        uCentral::RESTAPI_utils::field_to_json(Obj,"description",description);
        uCentral::RESTAPI_utils::field_to_json(Obj,"created",created);
        uCentral::RESTAPI_utils::field_to_json(Obj,"modified",modified);
        uCentral::RESTAPI_utils::field_to_json(Obj,"notes",notes);
    }

    bool ObjectInfo::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            uCentral::RESTAPI_utils::field_from_json(Obj,"id",id);
            uCentral::RESTAPI_utils::field_from_json(Obj,"name",name);
            uCentral::RESTAPI_utils::field_from_json(Obj,"description",description);
            uCentral::RESTAPI_utils::field_from_json(Obj,"created",created);
            uCentral::RESTAPI_utils::field_from_json(Obj,"modified",modified);
            uCentral::RESTAPI_utils::field_from_json(Obj,"notes",notes);
            return true;
        } catch(...) {

        }
        return false;
    }

    void ManagementPolicyEntry::to_json(Poco::JSON::Object &Obj) const {
        uCentral::RESTAPI_utils::field_to_json( Obj,"users",users);
        uCentral::RESTAPI_utils::field_to_json( Obj,"resources",resources);
        uCentral::RESTAPI_utils::field_to_json( Obj,"access",access);
        uCentral::RESTAPI_utils::field_to_json( Obj,"policy",policy);
    }

    bool ManagementPolicyEntry::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            uCentral::RESTAPI_utils::field_from_json( Obj,"users",users);
            uCentral::RESTAPI_utils::field_from_json( Obj,"resources",resources);
            uCentral::RESTAPI_utils::field_from_json( Obj,"access",access);
            uCentral::RESTAPI_utils::field_from_json( Obj,"policy",policy);
            return true;
        } catch(...) {

        }
        return false;
    }

    void ManagementPolicy::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        uCentral::RESTAPI_utils::field_to_json(Obj, "entries", entries);
    }

    bool ManagementPolicy::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            uCentral::RESTAPI_utils::field_from_json(Obj, "entries", entries);
            return true;
        } catch(...) {

        }
        return false;
    }

    void Entity::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        uCentral::RESTAPI_utils::field_to_json( Obj,"parent",parent);
        uCentral::RESTAPI_utils::field_to_json( Obj,"venues",venues);
        uCentral::RESTAPI_utils::field_to_json( Obj,"children",children);
        uCentral::RESTAPI_utils::field_to_json( Obj,"managers",managers);
        uCentral::RESTAPI_utils::field_to_json( Obj,"contacts",contacts);
        uCentral::RESTAPI_utils::field_to_json( Obj,"locations",locations);
        uCentral::RESTAPI_utils::field_to_json( Obj,"managementPolicy",managementPolicy);
    }

    bool Entity::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            uCentral::RESTAPI_utils::field_from_json( Obj,"parent",parent);
            uCentral::RESTAPI_utils::field_from_json( Obj,"venues",venues);
            uCentral::RESTAPI_utils::field_from_json( Obj,"children",children);
            uCentral::RESTAPI_utils::field_from_json( Obj,"managers",managers);
            uCentral::RESTAPI_utils::field_from_json( Obj,"contacts",contacts);
            uCentral::RESTAPI_utils::field_from_json( Obj,"locations",locations);
            uCentral::RESTAPI_utils::field_from_json( Obj,"managementPolicy",managementPolicy);
            return true;
        } catch(...) {

        }
        return false;
    }

    void DiGraphEntry::to_json(Poco::JSON::Object &Obj) const {
        uCentral::RESTAPI_utils::field_to_json( Obj,"parent",parent);
        uCentral::RESTAPI_utils::field_to_json( Obj,"child",child);
    }

    bool DiGraphEntry::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            uCentral::RESTAPI_utils::field_from_json( Obj,"parent",parent);
            uCentral::RESTAPI_utils::field_from_json( Obj,"child",child);
            return true;
        } catch (...) {

        }
        return false;
    }

    void Venue::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        uCentral::RESTAPI_utils::field_to_json( Obj,"parent",parent);
        uCentral::RESTAPI_utils::field_to_json( Obj,"owner",owner);
        uCentral::RESTAPI_utils::field_to_json( Obj,"children",children);
        uCentral::RESTAPI_utils::field_to_json( Obj,"managers",managers);
        uCentral::RESTAPI_utils::field_to_json( Obj,"devices",devices);
        uCentral::RESTAPI_utils::field_to_json( Obj,"topology",topology);
        uCentral::RESTAPI_utils::field_to_json( Obj,"parent",parent);
        uCentral::RESTAPI_utils::field_to_json( Obj,"design",design);
        uCentral::RESTAPI_utils::field_to_json( Obj,"managementPolicy",managementPolicy);
    }

    bool Venue::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            uCentral::RESTAPI_utils::field_from_json( Obj,"parent",parent);
            uCentral::RESTAPI_utils::field_from_json( Obj,"owner",owner);
            uCentral::RESTAPI_utils::field_from_json( Obj,"children",children);
            uCentral::RESTAPI_utils::field_from_json( Obj,"managers",managers);
            uCentral::RESTAPI_utils::field_from_json( Obj,"devices",devices);
            uCentral::RESTAPI_utils::field_from_json( Obj,"topology",topology);
            uCentral::RESTAPI_utils::field_from_json( Obj,"parent",parent);
            uCentral::RESTAPI_utils::field_from_json( Obj,"design",design);
            uCentral::RESTAPI_utils::field_from_json( Obj,"managementPolicy",managementPolicy);
            return true;
        } catch (...) {

        }
        return false;
    }

    void ManagementGroup::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        uCentral::RESTAPI_utils::field_to_json( Obj,"managers",managers);
        managementPolicy.to_json(Obj);
    }

    bool ManagementGroup::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            uCentral::RESTAPI_utils::field_from_json( Obj,"managers",managers);
            managementPolicy.from_json(Obj);
            return true;
        } catch (...) {

        }
        return false;
    }

    void Location::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        uCentral::RESTAPI_utils::field_to_json( Obj,"type",OpenWifi::ProvObjects::to_string(type));
        uCentral::RESTAPI_utils::field_to_json( Obj,"buildingName",buildingName);
        uCentral::RESTAPI_utils::field_to_json( Obj,"addressLines",addressLines);
        uCentral::RESTAPI_utils::field_to_json( Obj,"city",city);
        uCentral::RESTAPI_utils::field_to_json( Obj,"state",state);
        uCentral::RESTAPI_utils::field_to_json( Obj,"postal",postal);
        uCentral::RESTAPI_utils::field_to_json( Obj,"country",country);
        uCentral::RESTAPI_utils::field_to_json( Obj,"telephones",telephones);
        uCentral::RESTAPI_utils::field_to_json( Obj,"contact",contact);
    }

    bool Location::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            std::string tmp_type;
            uCentral::RESTAPI_utils::field_from_json( Obj,"type", tmp_type);
            type = location_from_string(tmp_type);
            uCentral::RESTAPI_utils::field_from_json( Obj,"buildingName",buildingName);
            uCentral::RESTAPI_utils::field_from_json( Obj,"addressLines",addressLines);
            uCentral::RESTAPI_utils::field_from_json( Obj,"city",city);
            uCentral::RESTAPI_utils::field_from_json( Obj,"state",state);
            uCentral::RESTAPI_utils::field_from_json( Obj,"postal",postal);
            uCentral::RESTAPI_utils::field_from_json( Obj,"country",country);
            uCentral::RESTAPI_utils::field_from_json( Obj,"telephones",telephones);
            uCentral::RESTAPI_utils::field_from_json( Obj,"contact",contact);
            return true;
        } catch (...) {

        }
        return false;
    }

    void Contact::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        uCentral::RESTAPI_utils::field_to_json( Obj,"type", to_string(type));
        uCentral::RESTAPI_utils::field_to_json( Obj,"title",title);
        uCentral::RESTAPI_utils::field_to_json( Obj,"salutation",salutation);
        uCentral::RESTAPI_utils::field_to_json( Obj,"firstname",firstname);
        uCentral::RESTAPI_utils::field_to_json( Obj,"lastname",lastname);
        uCentral::RESTAPI_utils::field_to_json( Obj,"initials",initials);
        uCentral::RESTAPI_utils::field_to_json( Obj,"visual",visual);
        uCentral::RESTAPI_utils::field_to_json( Obj,"mobiles",mobiles);
        uCentral::RESTAPI_utils::field_to_json( Obj,"phones",phones);
        uCentral::RESTAPI_utils::field_to_json( Obj,"location",location);
        uCentral::RESTAPI_utils::field_to_json( Obj,"primaryEmail",primaryEmail);
        uCentral::RESTAPI_utils::field_to_json( Obj,"secondaryEmail",secondaryEmail);
        uCentral::RESTAPI_utils::field_to_json( Obj,"accessPIN",accessPIN);
    }

    bool Contact::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            std::string tmp_type;
            uCentral::RESTAPI_utils::field_from_json( Obj,"type", tmp_type);
            type = contact_from_string(tmp_type);
            uCentral::RESTAPI_utils::field_from_json( Obj,"title",title);
            uCentral::RESTAPI_utils::field_from_json( Obj,"salutation",salutation);
            uCentral::RESTAPI_utils::field_from_json( Obj,"firstname",firstname);
            uCentral::RESTAPI_utils::field_from_json( Obj,"lastname",lastname);
            uCentral::RESTAPI_utils::field_from_json( Obj,"initials",initials);
            uCentral::RESTAPI_utils::field_from_json( Obj,"visual",visual);
            uCentral::RESTAPI_utils::field_from_json( Obj,"mobiles",mobiles);
            uCentral::RESTAPI_utils::field_from_json( Obj,"phones",phones);
            uCentral::RESTAPI_utils::field_from_json( Obj,"location",location);
            uCentral::RESTAPI_utils::field_from_json( Obj,"primaryEmail",primaryEmail);
            uCentral::RESTAPI_utils::field_from_json( Obj,"secondaryEmail",secondaryEmail);
            uCentral::RESTAPI_utils::field_from_json( Obj,"accessPIN",accessPIN);
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
        uCentral::RESTAPI_utils::field_to_json(Obj, "serialNumber", serialNumber);
        uCentral::RESTAPI_utils::field_to_json(Obj, "venue", venue);
        uCentral::RESTAPI_utils::field_to_json(Obj, "entity", entity);
        uCentral::RESTAPI_utils::field_to_json(Obj, "subEntity", subEntity);
        uCentral::RESTAPI_utils::field_to_json(Obj, "subVenue", subVenue);
        uCentral::RESTAPI_utils::field_to_json(Obj, "subscriber", subscriber);
        uCentral::RESTAPI_utils::field_to_json(Obj, "deviceType", deviceType);
        uCentral::RESTAPI_utils::field_to_json(Obj, "qrCode", qrCode);
    }

    bool InventoryTag::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            uCentral::RESTAPI_utils::field_from_json( Obj,"serialNumber",serialNumber);
            uCentral::RESTAPI_utils::field_from_json( Obj,"venue",venue);
            uCentral::RESTAPI_utils::field_from_json( Obj,"entity",entity);
            uCentral::RESTAPI_utils::field_from_json( Obj,"subEntity",subEntity);
            uCentral::RESTAPI_utils::field_from_json( Obj,"subVenue",subVenue);
            uCentral::RESTAPI_utils::field_from_json( Obj,"subscriber",subscriber);
            uCentral::RESTAPI_utils::field_from_json( Obj,"deviceType",deviceType);
            uCentral::RESTAPI_utils::field_from_json( Obj,"qrCode",qrCode);
            return true;
        } catch(...) {

        }
        return false;
    }

    void DeviceConfiguration::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        managementPolicy.to_json(Obj);
        uCentral::RESTAPI_utils::field_to_json( Obj,"deviceTypes",deviceTypes);
        uCentral::RESTAPI_utils::field_to_json( Obj,"configuration",configuration);
    }

    bool DeviceConfiguration::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            managementPolicy.from_json(Obj);
            uCentral::RESTAPI_utils::field_from_json( Obj,"deviceTypes",deviceTypes);
            uCentral::RESTAPI_utils::field_from_json( Obj,"configuration",configuration);
            return true;
        } catch(...) {

        }
        return false;
    }

    void Report::to_json(Poco::JSON::Object &Obj) const {
        uCentral::RESTAPI_utils::field_to_json(Obj, "snapshot", snapShot);
        uCentral::RESTAPI_utils::field_to_json(Obj, "devices", tenants);
    };

    void Report::reset() {
        tenants.clear();
    }

};
