//
// Created by stephane bourque on 2021-10-18.
//

#ifndef OWPROV_RESTAPI_DB_HELPERS_H
#define OWPROV_RESTAPI_DB_HELPERS_H

#include "RESTAPI_ProvObjects.h"

namespace OpenWifi {

    bool AddInventoryExtendedInfo(const ProvObjects::InventoryTag &T, Poco::JSON::Object &O);

}

#endif //OWPROV_RESTAPI_DB_HELPERS_H
