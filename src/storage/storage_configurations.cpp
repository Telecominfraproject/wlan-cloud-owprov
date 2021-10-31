//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//


#include "storage_configurations.h"
#include "framework/OpenWifiTypes.h"
#include "framework/MicroService.h"
#include "RESTObjects/RESTAPI_SecurityObjects.h"
#include "StorageService.h"

namespace OpenWifi {

    static  ORM::FieldVec    ConfigurationDB_Fields{
        // object info
        ORM::Field{"id",64, true},
        ORM::Field{"name",ORM::FieldType::FT_TEXT},
        ORM::Field{"description",ORM::FieldType::FT_TEXT},
        ORM::Field{"notes",ORM::FieldType::FT_TEXT},
        ORM::Field{"created",ORM::FieldType::FT_BIGINT},
        ORM::Field{"modified",ORM::FieldType::FT_BIGINT},
        ORM::Field{"managementPolicy",ORM::FieldType::FT_TEXT},
        ORM::Field{"deviceTypes",ORM::FieldType::FT_TEXT},
        ORM::Field{"configuration",ORM::FieldType::FT_TEXT},
        ORM::Field{"inUse",ORM::FieldType::FT_TEXT},
        ORM::Field{"variables",ORM::FieldType::FT_TEXT},
        ORM::Field{"rrm",ORM::FieldType::FT_TEXT},
        ORM::Field{"tags",ORM::FieldType::FT_TEXT},
        ORM::Field{"firmwareUpgrade",ORM::FieldType::FT_TEXT},
        ORM::Field{"firmwareRCOnly",ORM::FieldType::FT_INT}
    };

    static  ORM::IndexVec    ConfigurationDB_Indexes{
        { std::string("configuration_name_index"),
          ORM::IndexEntryVec{
            {std::string("name"),
             ORM::Indextype::ASC} } }
    };

    ConfigurationDB::ConfigurationDB( OpenWifi::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L) :
        DB(T, "configurations", ConfigurationDB_Fields, ConfigurationDB_Indexes, P, L, "cfg") {}

        static bool AddDevicesFromVenue( const std::string & UUID, std::set<std::string> & Devices) {
        ProvObjects::Venue V;
        if(!StorageService()->VenueDB().GetRecord("id",UUID,V))
            return false;
        for(const auto &j:V.devices) {
            ProvObjects::InventoryTag   T;
            if(!StorageService()->InventoryDB().GetRecord("id",j,T))
                continue;
            Devices.insert(T.serialNumber);
        }

        for(const auto &j:V.children)
            AddDevicesFromVenue(j,Devices);
        return true;
    }

    static bool AddDevicesFromEntity( const std::string & UUID, std::set<std::string> & Devices) {
        ProvObjects::Entity E;
        if(!StorageService()->EntityDB().GetRecord("id",UUID,E))
            return false;
        for(const auto &j:E.devices) {
            ProvObjects::InventoryTag   T;
            if(!StorageService()->InventoryDB().GetRecord("id",j,T))
                continue;
            Devices.insert(T.serialNumber);
        }
        for(const auto &j:E.children)
            AddDevicesFromEntity(j, Devices);
        for(const auto &j:E.venues)
            AddDevicesFromVenue(j, Devices);
        return true;
    }

    bool ConfigurationDB::GetListOfAffectedDevices(const Types::UUID_t & ConfigUUID, Types::UUIDvec_t & DeviceSerialNumbers ) {
        //  find all the places where this configuration is used
        //  for each of them get the devices they oversee
        ProvObjects::DeviceConfiguration    Config;
        if(!GetRecord("id",ConfigUUID,Config)) {
            return false;
        }

        if(Config.inUse.empty())
            return true;

        std::set<std::string>   SerialNumbers;
        for(const auto &i:Config.inUse) {
            auto Tokens = Poco::StringTokenizer(i,":");
            if(Tokens.count()!=2)
                continue;
            if(Tokens[0] == "ent") {
                AddDevicesFromEntity(Tokens[1], SerialNumbers);
            } else if (Tokens[0] == "ven") {
                AddDevicesFromVenue(Tokens[1], SerialNumbers);
            } else if (Tokens[0] == "inv") {
                ProvObjects::InventoryTag   T;
                if(!StorageService()->InventoryDB().GetRecord("id",Tokens[1],T))
                    continue;
                SerialNumbers.insert(T.serialNumber);
            }
        }

        for(const auto &i:SerialNumbers)
            DeviceSerialNumbers.push_back(i);

        return true;
    }

}

template<> void ORM::DB<    OpenWifi::ConfigurationDBRecordType, OpenWifi::ProvObjects::DeviceConfiguration>::Convert(OpenWifi::ConfigurationDBRecordType &In, OpenWifi::ProvObjects::DeviceConfiguration &Out) {
    Out.info.id = In.get<0>();
    Out.info.name = In.get<1>();
    Out.info.description = In.get<2>();
    Out.info.notes = OpenWifi::RESTAPI_utils::to_object_array<OpenWifi::SecurityObjects::NoteInfo>(In.get<3>());
    Out.info.created = In.get<4>();
    Out.info.modified = In.get<5>();
    Out.managementPolicy = In.get<6>();
    OpenWifi::Types::from_string(In.get<7>(), Out.deviceTypes);
    Out.configuration = OpenWifi::RESTAPI_utils::to_object_array<OpenWifi::ProvObjects::DeviceConfigurationElement>(In.get<8>());
    OpenWifi::Types::from_string(In.get<9>(), Out.inUse);
    OpenWifi::Types::from_string(In.get<10>(), Out.variables);
    Out.rrm = In.get<11>();
    Out.info.tags = OpenWifi::RESTAPI_utils::to_taglist(In.get<12>());
    Out.firmwareUpgrade = In.get<13>();
    Out.firmwareRCOnly = In.get<14>();
}

template<> void ORM::DB<    OpenWifi::ConfigurationDBRecordType, OpenWifi::ProvObjects::DeviceConfiguration>::Convert(OpenWifi::ProvObjects::DeviceConfiguration &In, OpenWifi::ConfigurationDBRecordType &Out) {
    Out.set<0>(In.info.id);
    Out.set<1>(In.info.name);
    Out.set<2>(In.info.description);
    Out.set<3>(OpenWifi::RESTAPI_utils::to_string(In.info.notes));
    Out.set<4>(In.info.created);
    Out.set<5>(In.info.modified);
    Out.set<6>(In.managementPolicy);
    Out.set<7>(OpenWifi::Types::to_string(In.deviceTypes));
    Out.set<8>(OpenWifi::RESTAPI_utils::to_string(In.configuration));
    Out.set<9>(OpenWifi::Types::to_string(In.inUse));
    Out.set<10>(OpenWifi::Types::to_string(In.variables));
    Out.set<11>(In.rrm);
    Out.set<12>(OpenWifi::RESTAPI_utils::to_string(In.info.tags));
    Out.set<13>(In.firmwareUpgrade);
    Out.set<14>(In.firmwareRCOnly);
}
