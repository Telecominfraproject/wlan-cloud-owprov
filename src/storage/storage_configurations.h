//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//


#ifndef OWPROV_STORAGE_CONFIGURATIONS_H
#define OWPROV_STORAGE_CONFIGURATIONS_H


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
    std::string,
    std::string,
    std::string,
    uint32_t
    > ConfigurationDBRecordType;

    class ConfigurationDB : public ORM::DB<ConfigurationDBRecordType, ProvObjects::DeviceConfiguration> {
    public:
        ConfigurationDB( OpenWifi::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L);
    private:
    };
}


#endif //OWPROV_STORAGE_CONFIGURATIONS_H
