//
// Created by stephane bourque on 2021-11-09.
//

#include "storage_maps.h"

namespace OpenWifi {

    static  ORM::FieldVec    MapsDB_Fields{
        // object info
        ORM::Field{"id",64, true},
        ORM::Field{"name",ORM::FieldType::FT_TEXT},
        ORM::Field{"description",ORM::FieldType::FT_TEXT},
        ORM::Field{"notes",ORM::FieldType::FT_TEXT},
        ORM::Field{"created",ORM::FieldType::FT_BIGINT},
        ORM::Field{"modified",ORM::FieldType::FT_BIGINT},
        ORM::Field{"tags",ORM::FieldType::FT_TEXT},
        ORM::Field{"data",ORM::FieldType::FT_TEXT},
        ORM::Field{"creator",ORM::FieldType::FT_TEXT},
        ORM::Field{"visibility",ORM::FieldType::FT_TEXT},
        ORM::Field{"access",ORM::FieldType::FT_TEXT},
        ORM::Field{"entity",ORM::FieldType::FT_TEXT},
        ORM::Field{"managementPolicy",ORM::FieldType::FT_TEXT},
        ORM::Field{"venue",ORM::FieldType::FT_TEXT}
        };

    static  ORM::IndexVec    MapsDB_Indexes{
        { std::string("maps_name_index"),
          ORM::IndexEntryVec{
            {std::string("name"),
             ORM::Indextype::ASC} } }
    };

    MapDB::MapDB( OpenWifi::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L) :
    DB(T, "maps", MapsDB_Fields, MapsDB_Indexes, P, L, "map") {}

    bool MapDB::Upgrade([[maybe_unused]] uint32_t from, uint32_t &to) {
        std::vector<std::string> Statements{
            "alter table " + TableName_ + " add column managementPolicy text;",
            "alter table " + TableName_ + " add column entity text;",
            "alter table " + TableName_ + " add column venue text;"
        };
        RunScript(Statements);
        to = 2;
        return true;
    }

}

template<> void ORM::DB<    OpenWifi::MapDBRecordType, OpenWifi::ProvObjects::Map>::Convert(const OpenWifi::MapDBRecordType &In, OpenWifi::ProvObjects::Map &Out) {
    Out.info.id = In.get<0>();
    Out.info.name = In.get<1>();
    Out.info.description = In.get<2>();
    Out.info.notes = OpenWifi::RESTAPI_utils::to_object_array<OpenWifi::SecurityObjects::NoteInfo>(In.get<3>());
    Out.info.created = In.get<4>();
    Out.info.modified = In.get<5>();
    Out.info.tags = OpenWifi::RESTAPI_utils::to_taglist(In.get<6>());
    Out.data = In.get<7>();
    Out.creator = In.get<8>();
    Out.visibility = In.get<9>();
    Out.access = OpenWifi::RESTAPI_utils::to_object<OpenWifi::ProvObjects::ObjectACLList>(In.get<10>());
    Out.entity = In.get<11>();
    Out.managementPolicy = In.get<12>();
    Out.venue = In.get<13>();
}

template<> void ORM::DB<    OpenWifi::MapDBRecordType, OpenWifi::ProvObjects::Map>::Convert(const OpenWifi::ProvObjects::Map &In, OpenWifi::MapDBRecordType &Out) {
    Out.set<0>(In.info.id);
    Out.set<1>(In.info.name);
    Out.set<2>(In.info.description);
    Out.set<3>(OpenWifi::RESTAPI_utils::to_string(In.info.notes));
    Out.set<4>(In.info.created);
    Out.set<5>(In.info.modified);
    Out.set<6>(OpenWifi::RESTAPI_utils::to_string(In.info.tags));
    Out.set<7>(In.data);
    Out.set<8>(In.creator);
    Out.set<9>(In.visibility);
    Out.set<10>(OpenWifi::RESTAPI_utils::to_string(In.access));
    Out.set<11>(In.entity);
    Out.set<12>(In.managementPolicy);
    Out.set<13>(In.venue);
}
