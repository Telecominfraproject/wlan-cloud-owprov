//
// Created by stephane bourque on 2021-10-18.
//

#ifndef OWPROV_RESTAPI_DB_HELPERS_H
#define OWPROV_RESTAPI_DB_HELPERS_H

#include "RESTAPI_ProvObjects.h"

namespace OpenWifi {

    bool AddInventoryExtendedInfo(const ProvObjects::InventoryTag &T, Poco::JSON::Object &O);
    bool AddLocationExtendedInfo(const ProvObjects::Location & T, Poco::JSON::Object &O);
    bool AddContactExtendedInfo(const ProvObjects::Contact &T, Poco::JSON::Object &O);
    bool AddEntityExtendedInfo(const ProvObjects::Entity &T, Poco::JSON::Object &O);
    bool AddVenueExtendedInfo(const ProvObjects::Venue &T, Poco::JSON::Object &O);
    bool AddManagementPolicyExtendedInfo(const ProvObjects::ManagementPolicy &T, Poco::JSON::Object &O);
    bool AddManagementRoleExtendedInfo(const ProvObjects::ManagementRole &T, Poco::JSON::Object &O);
}

#endif //OWPROV_RESTAPI_DB_HELPERS_H
