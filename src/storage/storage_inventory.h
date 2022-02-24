//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#pragma once

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
        std::string,
        std::string,
        std::string,
        std::string
    > InventoryDBRecordType;

    class InventoryDB : public ORM::DB<InventoryDBRecordType, ProvObjects::InventoryTag> {
        public:
            InventoryDB( OpenWifi::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L);
            bool CreateFromConnection(const std::string & SerialNumber, const std::string & ConnectionInfo,
                                      const std::string & DeviceType, const std::string &Locale );
            bool FindFirmwareOptions(std::string  & SerialNumber, ProvObjects::FIRMWARE_UPGRADE_RULES & Rules);
            static bool FindFirmwareOptionsForEntity(const std::string & EntityUUID, ProvObjects::FIRMWARE_UPGRADE_RULES & Rules);
            static bool FindFirmwareOptionsForVenue(const std::string & VenueUUID, ProvObjects::FIRMWARE_UPGRADE_RULES & Rules);
            void InitializeSerialCache();
            bool GetRRMDeviceList(Types::UUIDvec_t & DeviceList);
            bool LookForRRM( const ProvObjects::InventoryTag &T);
            bool LookForRRMInVenue(const std::string &Venue);
            bool LookForRRMInEntity(const std::string &Entity);
            inline uint32_t Version() override {
                return 1;
            }
            bool Upgrade(uint32_t from, uint32_t &to) override;

        private:
    };
}
