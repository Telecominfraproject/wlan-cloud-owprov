//
// Created by stephane bourque on 2021-08-16.
//

#include "storage_location.h"
#include "Utils.h"
#include "uCentralTypes.h"
#include "RESTAPI_utils.h"
#include "RESTAPI_SecurityObjects.h"

namespace OpenWifi {

    static  ORM::FieldVec    LocationDB_Fields{
        // object info
        ORM::Field{"id",64, true},
        ORM::Field{"name",ORM::FieldType::FT_TEXT},
        ORM::Field{"description",ORM::FieldType::FT_TEXT},
        ORM::Field{"notes",ORM::FieldType::FT_TEXT},
        ORM::Field{"created",ORM::FieldType::FT_BIGINT},
        ORM::Field{"modified",ORM::FieldType::FT_BIGINT},
        ORM::Field{"type",ORM::FieldType::FT_TEXT},
        ORM::Field{"buildingName",ORM::FieldType::FT_TEXT},
        ORM::Field{"addressLines",ORM::FieldType::FT_TEXT},
        ORM::Field{"city",ORM::FieldType::FT_TEXT},
        ORM::Field{"state",ORM::FieldType::FT_TEXT},
        ORM::Field{"postal",ORM::FieldType::FT_TEXT},
        ORM::Field{"country",ORM::FieldType::FT_TEXT},
        ORM::Field{"telephones",ORM::FieldType::FT_TEXT},
        ORM::Field{"contact",ORM::FieldType::FT_TEXT}
    };

    static  ORM::IndexVec    LocationDB_Indexes{
        { std::string("location_name_index"),
          ORM::IndexEntryVec{
            {std::string("name"),
             ORM::Indextype::ASC} } }
    };

    LocationDB::LocationDB( ORM::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L) :
    DB(T, "locations", LocationDB_Fields, LocationDB_Indexes, P, L) {}

}

template<> void ORM::DB<    OpenWifi::LocationDBRecordType, OpenWifi::ProvObjects::Location>::Convert(OpenWifi::LocationDBRecordType &In, OpenWifi::ProvObjects::Location &Out) {
    Out.info.id = In.get<0>();
    Out.info.name = In.get<1>();
    Out.info.description = In.get<2>();
    Out.info.notes = uCentral::RESTAPI_utils::to_object_array<uCentral::SecurityObjects::NoteInfo>(In.get<3>());
    Out.info.created = In.get<4>();
    Out.info.modified = In.get<5>();
    Out.type = OpenWifi::ProvObjects::location_from_string(In.get<6>());
    Out.buildingName = In.get<7>();
    uCentral::Types::from_string(In.get<8>(), Out.addressLines);
    Out.city = In.get<9>();
    Out.state = In.get<10>();
    Out.postal = In.get<11>();
    Out.country = In.get<12>();
    uCentral::Types::from_string(In.get<13>(), Out.telephones);
    Out.contact = In.get<14>();
}

template<> void ORM::DB<    OpenWifi::LocationDBRecordType, OpenWifi::ProvObjects::Location>::Convert(OpenWifi::ProvObjects::Location &In, OpenWifi::LocationDBRecordType &Out) {
    Out.set<0>(In.info.id);
    Out.set<1>(In.info.name);
    Out.set<2>(In.info.description);
    Out.set<3>(uCentral::RESTAPI_utils::to_string(In.info.notes));
    Out.set<4>(In.info.created);
    Out.set<5>(In.info.modified);
    Out.set<6>(OpenWifi::ProvObjects::to_string(In.type));
    Out.set<7>(In.buildingName);
    Out.set<8>(uCentral::Types::to_string(In.addressLines));
    Out.set<9>(In.city);
    Out.set<10>(In.state);
    Out.set<11>(In.postal);
    Out.set<12>(In.country);
    Out.set<13>(uCentral::Types::to_string(In.telephones));
    Out.set<14>(In.contact);
}
