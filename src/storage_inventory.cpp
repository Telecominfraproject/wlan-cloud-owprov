//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//


#include "storage_inventory.h"
#include "Utils.h"
#include "OpenWifiTypes.h"
#include "RESTAPI_utils.h"
#include "RESTAPI_SecurityObjects.h"

namespace OpenWifi {

    static  ORM::FieldVec    InventoryDB_Fields{
        // object info
        ORM::Field{"id",64, true},
        ORM::Field{"name",ORM::FieldType::FT_TEXT},
        ORM::Field{"description",ORM::FieldType::FT_TEXT},
        ORM::Field{"notes",ORM::FieldType::FT_TEXT},
        ORM::Field{"created",ORM::FieldType::FT_BIGINT},
        ORM::Field{"modified",ORM::FieldType::FT_BIGINT},
        ORM::Field{"serialNumber",ORM::FieldType::FT_TEXT},
        ORM::Field{"venue",ORM::FieldType::FT_TEXT},
        ORM::Field{"entity",ORM::FieldType::FT_TEXT},
        ORM::Field{"subscriber",ORM::FieldType::FT_TEXT},
        ORM::Field{"deviceType",ORM::FieldType::FT_TEXT},
        ORM::Field{"qrcode",ORM::FieldType::FT_TEXT},
        ORM::Field{"geocode",ORM::FieldType::FT_TEXT},
        ORM::Field{"location",ORM::FieldType::FT_TEXT},
        ORM::Field{"contact",ORM::FieldType::FT_TEXT}
    };

    static  ORM::IndexVec    InventoryDB_Indexes{
        { std::string("inventory_name_index"),
          ORM::IndexEntryVec{
            {std::string("name"),
             ORM::Indextype::ASC} } },
         { std::string("inventory_serial_index"),
           ORM::IndexEntryVec{
            {std::string("serialNumber"),
             ORM::Indextype::ASC} } }
    };

    InventoryDB::InventoryDB( ORM::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L) :
        DB(T, "inventory", InventoryDB_Fields, InventoryDB_Indexes, P, L, "inv") {}

}

template<> void ORM::DB<    OpenWifi::InventoryDBRecordType, OpenWifi::ProvObjects::InventoryTag>::Convert(OpenWifi::InventoryDBRecordType &In, OpenWifi::ProvObjects::InventoryTag &Out) {
    Out.info.id = In.get<0>();
    Out.info.name = In.get<1>();
    Out.info.description = In.get<2>();
    Out.info.notes = OpenWifi::RESTAPI_utils::to_object_array<OpenWifi::SecurityObjects::NoteInfo>(In.get<3>());
    Out.info.created = In.get<4>();
    Out.info.modified = In.get<5>();
    Out.serialNumber = In.get<6>();
    Out.venue = In.get<7>();
    Out.entity = In.get<8>();
    Out.subscriber = In.get<9>();
    Out.deviceType = In.get<10>();
    Out.qrCode = In.get<11>();
    Out.geoCode = In.get<12>();
    Out.location = In.get<13>();
    Out.contact = In.get<14>();
}

template<> void ORM::DB<    OpenWifi::InventoryDBRecordType, OpenWifi::ProvObjects::InventoryTag>::Convert(OpenWifi::ProvObjects::InventoryTag &In, OpenWifi::InventoryDBRecordType &Out) {
    Out.set<0>(In.info.id);
    Out.set<1>(In.info.name);
    Out.set<2>(In.info.description);
    Out.set<3>(OpenWifi::RESTAPI_utils::to_string(In.info.notes));
    Out.set<4>(In.info.created);
    Out.set<5>(In.info.modified);
    Out.set<6>(In.serialNumber);
    Out.set<7>(In.venue);
    Out.set<8>(In.entity);
    Out.set<9>(In.subscriber);
    Out.set<10>(In.deviceType);
    Out.set<11>(In.qrCode);
    Out.set<12>(In.geoCode);
    Out.set<13>(In.location);
    Out.set<14>(In.contact);
}
