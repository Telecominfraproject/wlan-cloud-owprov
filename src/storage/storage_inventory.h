//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//


#ifndef OWPROV_STORAGE_INVENTORY_H
#define OWPROV_STORAGE_INVENTORY_H

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
        std::string,
        std::string,
        std::string,
        std::string,
        std::string
    > InventoryDBRecordType;

    class InventoryDB : public ORM::DB<InventoryDBRecordType, ProvObjects::InventoryTag> {
        public:
            InventoryDB( OpenWifi::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L);
            bool CreateFromConnection(const std::string & SerialNumber, const std::string & ConnectionInfo, const std::string & DeviceType);
            bool FindFirmwareOptions(std::string  & SerialNumber, std::string &firmwareUpgrade, bool &firmwareRCOnly);
            static bool FindFirmwareOptionsForEntity(const std::string & EntityUUID, std::string &firmwareUpgrade, bool &firmwareRCOnly);
            static bool FindFirmwareOptionsForVenue(const std::string & VenueUUID, std::string &firmwareUpgrade, bool &firmwareRCOnly);
            void InitializeSerialCache();
        private:
    };
}

#endif //OWPROV_STORAGE_INVENTORY_H