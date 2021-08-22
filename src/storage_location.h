//
// Created by stephane bourque on 2021-08-16.
//

#ifndef OWPROV_STORAGE_LOCATION_H
#define OWPROV_STORAGE_LOCATION_H

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
        std::string,
        std::string,
        std::string,
        std::string,
        std::string,
        std::string,
        std::string,
        std::string,
        std::string
    > LocationDBRecordType;

    class LocationDB : public ORM::DB<LocationDBRecordType, ProvObjects::Location> {
    public:
        LocationDB( ORM::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L);
    private:
    };
}


#endif //OWPROV_STORAGE_LOCATION_H
