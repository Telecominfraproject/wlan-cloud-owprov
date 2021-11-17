//
// Created by stephane bourque on 2021-11-09.
//

#ifndef OWPROV_STORAGE_MAPS_H
#define OWPROV_STORAGE_MAPS_H

#include "framework/orm.h"
#include "RESTObjects/RESTAPI_ProvObjects.h"

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
        std::string
    > MapDBRecordType;

    class MapDB : public ORM::DB<MapDBRecordType, ProvObjects::Map> {
    public:
        MapDB( OpenWifi::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L);
    private:
    };
}


#endif //OWPROV_STORAGE_MAPS_H