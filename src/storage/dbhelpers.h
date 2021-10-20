//
// Created by stephane bourque on 2021-08-17.
//

#ifndef OWPROV_DBHELPERS_H
#define OWPROV_DBHELPERS_H

#include "storage/storage_entity.h"
#include "storage/storage_venue.h"
#include "storage/storage_inventory.h"
#include "storage/storage_contact.h"
#include "storage/storage_location.h"
#include "storage/storage_policies.h"

namespace OpenWifi {

    template <typename DB> bool AddChild(DB &db, const std::string &parent_uuid, const std::string &child_uuid) {
    }

        // DB.getrecord("id",parent_uuid,P);
        // P.children.push_back(child);
        // DB.updaterecord(Parent_uuid,P);
    }

}




#endif //OWPROV_DBHELPERS_H
