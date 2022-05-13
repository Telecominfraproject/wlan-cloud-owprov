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
        ORM::Field{"deviceRules",ORM::FieldType::FT_TEXT},
        ORM::Field{"tags",ORM::FieldType::FT_TEXT},
        ORM::Field{"subscriberOnly",ORM::FieldType::FT_BOOLEAN},
        ORM::Field{"entity",ORM::FieldType::FT_TEXT},
        ORM::Field{"venue",ORM::FieldType::FT_TEXT},
        ORM::Field{"subscriber",ORM::FieldType::FT_TEXT}
    };

    static  ORM::IndexVec    ConfigurationDB_Indexes{
        { std::string("configuration_name_index"),
          ORM::IndexEntryVec{
            {std::string("name"),
             ORM::Indextype::ASC} } }
    };

    bool ConfigurationDB::Upgrade([[maybe_unused]] uint32_t from, uint32_t &to) {
        std::vector<std::string> Statements{
            "alter table " + TableName_ + " add column subscriberOnly BOOLEAN;",
            "alter table " + TableName_ + " add column entity TEXT;",
            "alter table " + TableName_ + " add column subscriber TEXT;",
            "alter table " + TableName_ + " add column venue TEXT;",
            "alter table " + TableName_ + " add column deviceRules TEXT;"
        };
        RunScript(Statements);
        to = 2;
        return true;
    }

    ConfigurationDB::ConfigurationDB( OpenWifi::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L) :
        DB(T, "configurations", ConfigurationDB_Fields, ConfigurationDB_Indexes, P, L, "cfg") {}

    static bool AddIfAffected( const std::string & UUID,const std::vector<std::string> & DeviceTypes, std::set<std::string> & Devices ) {
        ProvObjects::InventoryTag   T;
        if(!StorageService()->InventoryDB().GetRecord("id",UUID,T))
            return false;

        for(const auto &i:DeviceTypes) {
            if(i=="*" || i==T.deviceType) {
                Devices.insert(T.serialNumber);
                return true;
            }
        }
        return false;
    }

    static bool AddDevicesFromVenue( const std::string & UUID, const std::vector<std::string> & DeviceTypes, std::set<std::string> & Devices) {
        ProvObjects::Venue V;
        if(!StorageService()->VenueDB().GetRecord("id",UUID,V))
            return false;
        for(const auto &j:V.devices) {
            AddIfAffected(j, DeviceTypes, Devices);
        }
        for(const auto &j:V.children)
            AddDevicesFromVenue(j,DeviceTypes,Devices);
        return true;
    }

    static bool AddDevicesFromEntity( const std::string & UUID, const std::vector<std::string> & DeviceTypes, std::set<std::string> & Devices) {
        ProvObjects::Entity E;
        if(!StorageService()->EntityDB().GetRecord("id",UUID,E))
            return false;
        for(const auto &j:E.devices) {
            AddIfAffected(j, DeviceTypes, Devices);
        }
        for(const auto &j:E.children)
            AddDevicesFromEntity(j,DeviceTypes, Devices);
        for(const auto &j:E.venues)
            AddDevicesFromVenue(j,DeviceTypes, Devices);
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
        auto DeviceTypes = Config.deviceTypes;
        for(const auto &i:Config.inUse) {
            auto Tokens = Poco::StringTokenizer(i,":");
            if(Tokens.count()!=2)
                continue;
            if(Tokens[0] == "ent") {
                AddDevicesFromEntity(Tokens[1], DeviceTypes, SerialNumbers);
            } else if (Tokens[0] == "ven") {
                AddDevicesFromVenue(Tokens[1], DeviceTypes, SerialNumbers);
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

template<> void ORM::DB<    OpenWifi::ConfigurationDBRecordType, OpenWifi::ProvObjects::DeviceConfiguration>::Convert(const OpenWifi::ConfigurationDBRecordType &In, OpenWifi::ProvObjects::DeviceConfiguration &Out) {
    Out.info.id = In.get<0>();
    Out.info.name = In.get<1>();
    Out.info.description = In.get<2>();
    Out.info.notes = OpenWifi::RESTAPI_utils::to_object_array<OpenWifi::SecurityObjects::NoteInfo>(In.get<3>());
    Out.info.created = In.get<4>();
    Out.info.modified = In.get<5>();
    Out.managementPolicy = In.get<6>();
    Out.deviceTypes = OpenWifi::RESTAPI_utils::to_object_array(In.get<7>());
    Out.configuration = OpenWifi::RESTAPI_utils::to_object_array<OpenWifi::ProvObjects::DeviceConfigurationElement>(In.get<8>());
    Out.inUse = OpenWifi::RESTAPI_utils::to_object_array(In.get<9>());
    Out.variables = OpenWifi::RESTAPI_utils::to_object_array(In.get<10>());
    Out.deviceRules = OpenWifi::RESTAPI_utils::to_object<OpenWifi::ProvObjects::DeviceRules>(In.get<11>());
    Out.info.tags = OpenWifi::RESTAPI_utils::to_taglist(In.get<12>());
    Out.subscriberOnly = In.get<13>();
    Out.entity = In.get<14>();
    Out.venue = In.get<15>();
    Out.subscriber = In.get<16>();
}

template<> void ORM::DB<    OpenWifi::ConfigurationDBRecordType, OpenWifi::ProvObjects::DeviceConfiguration>::Convert(const OpenWifi::ProvObjects::DeviceConfiguration &In, OpenWifi::ConfigurationDBRecordType &Out) {
    Out.set<0>(In.info.id);
    Out.set<1>(In.info.name);
    Out.set<2>(In.info.description);
    Out.set<3>(OpenWifi::RESTAPI_utils::to_string(In.info.notes));
    Out.set<4>(In.info.created);
    Out.set<5>(In.info.modified);
    Out.set<6>(In.managementPolicy);
    Out.set<7>(OpenWifi::RESTAPI_utils::to_string(In.deviceTypes));
    Out.set<8>(OpenWifi::RESTAPI_utils::to_string(In.configuration));
    Out.set<9>(OpenWifi::RESTAPI_utils::to_string(In.inUse));
    Out.set<10>(OpenWifi::RESTAPI_utils::to_string(In.variables));
    Out.set<11>(OpenWifi::RESTAPI_utils::to_string(In.deviceRules));
    Out.set<12>(OpenWifi::RESTAPI_utils::to_string(In.info.tags));
    Out.set<13>(In.subscriberOnly);
    Out.set<14>(In.entity);
    Out.set<15>(In.venue);
    Out.set<16>(In.subscriber);
}
