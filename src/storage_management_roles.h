//
// Created by stephane bourque on 2021-08-20.
//

#ifndef OWPROV_STORAGE_MANAGEMENT_ROLES_H
#define OWPROV_STORAGE_MANAGEMENT_ROLES_H

#include "orm.h"
#include "RESTAPI_ProvObjects.h"

namespace OpenWifi {
    typedef Poco::Tuple<
    std::string,
    std::string,
    std::string,
    std::string,
    uint64_t,
    uint64_t,
    std::string,
    std::string,
    std::string
    > ManagementRoleDBRecordType;

    class ManagementRoleDB : public ORM::DB<ManagementRoleDBRecordType, ProvObjects::Location> {
    public:
        ManagementRoleDB( ORM::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L);
    private:
    };
}


#endif //OWPROV_STORAGE_MANAGEMENT_ROLES_H
