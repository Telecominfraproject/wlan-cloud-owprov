//
// Created by stephane bourque on 2021-08-16.
//

#ifndef OWPROV_STORAGE_POLICIES_H
#define OWPROV_STORAGE_POLICIES_H

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
        std::string
    > PolicyDBRecordType;

    class PolicyDB : public ORM::DB<PolicyDBRecordType, ProvObjects::ManagementPolicy> {
    public:
        PolicyDB( ORM::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L);
    private:
    };
}


#endif //OWPROV_STORAGE_POLICIES_H
