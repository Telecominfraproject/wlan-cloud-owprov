//
// Created by stephane bourque on 2021-08-16.
//

#include "storage_venue.h"
#include "OpenWifiTypes.h"
#include "RESTAPI_utils.h"
#include "RESTAPI_SecurityObjects.h"

namespace OpenWifi {

    static  ORM::FieldVec    VenueDB_Fields{
        // object info
        ORM::Field{"id",64, true},
        ORM::Field{"name",ORM::FieldType::FT_TEXT},
        ORM::Field{"description",ORM::FieldType::FT_TEXT},
        ORM::Field{"notes",ORM::FieldType::FT_TEXT},
        ORM::Field{"created",ORM::FieldType::FT_BIGINT},
        ORM::Field{"modified",ORM::FieldType::FT_BIGINT},
        ORM::Field{"entity",ORM::FieldType::FT_TEXT},
        ORM::Field{"parent",ORM::FieldType::FT_TEXT},
        ORM::Field{"children",ORM::FieldType::FT_TEXT},
        ORM::Field{"managementPolicy",ORM::FieldType::FT_TEXT},
        ORM::Field{"devices",ORM::FieldType::FT_TEXT},
        ORM::Field{"topology",ORM::FieldType::FT_TEXT},
        ORM::Field{"design",ORM::FieldType::FT_TEXT}
    };

    static  ORM::IndexVec    VenueDB_Indexes{
        { std::string("venue_name_index"),
          ORM::IndexEntryVec{
            {std::string("name"),
             ORM::Indextype::ASC} } }
    };

    VenueDB::VenueDB( ORM::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L) :
        DB(T, "venues", VenueDB_Fields, VenueDB_Indexes, P, L) {}

}

template<> void ORM::DB<    OpenWifi::VenueDBRecordType, OpenWifi::ProvObjects::Venue>::Convert(OpenWifi::VenueDBRecordType &In, OpenWifi::ProvObjects::Venue &Out) {
    Out.info.id = In.get<0>();
    Out.info.name = In.get<1>();
    Out.info.description = In.get<2>();
    Out.info.notes = OpenWifi::RESTAPI_utils::to_object_array<OpenWifi::SecurityObjects::NoteInfo>(In.get<3>());
    Out.info.created = In.get<4>();
    Out.info.modified = In.get<5>();
    Out.entity = In.get<6>();
    Out.parent = In.get<7>();
    OpenWifi::Types::from_string(In.get<8>(), Out.children);
    OpenWifi::Types::from_string(In.get<9>(), Out.devices);
    Out.topology = OpenWifi::RESTAPI_utils::to_object_array<OpenWifi::ProvObjects::DiGraphEntry>(In.get<10>());
    Out.design = In.get<11>();
}

template<> void ORM::DB<    OpenWifi::VenueDBRecordType, OpenWifi::ProvObjects::Venue>::Convert(OpenWifi::ProvObjects::Venue &In, OpenWifi::VenueDBRecordType &Out) {
    Out.set<0>(In.info.id);
    Out.set<1>(In.info.name);
    Out.set<2>(In.info.description);
    Out.set<3>(OpenWifi::RESTAPI_utils::to_string(In.info.notes));
    Out.set<4>(In.info.created);
    Out.set<5>(In.info.modified);
    Out.set<6>(In.entity);
    Out.set<7>(In.parent);
    Out.set<8>(OpenWifi::Types::to_string(In.children));
    Out.set<9>(OpenWifi::Types::to_string(In.devices));
    Out.set<10>(OpenWifi::RESTAPI_utils::to_string(In.topology));
    Out.set<11>(In.design);
}
