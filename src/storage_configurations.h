//
// Created by stephane bourque on 2021-08-22.
//

#ifndef OWPROV_STORAGE_CONFIGURATIONS_H
#define OWPROV_STORAGE_CONFIGURATIONS_H


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
    std::string
    > ConfigurationDBRecordType;

    class ConfigurationDB : public ORM::DB<ConfigurationDBRecordType, ProvObjects::DeviceConfiguration> {
    public:
        ConfigurationDB( ORM::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L);
    private:
    };
}


#endif //OWPROV_STORAGE_CONFIGURATIONS_H
