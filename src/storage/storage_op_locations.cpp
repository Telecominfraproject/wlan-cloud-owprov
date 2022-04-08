//
// Created by stephane bourque on 2022-04-07.
//

#include "storage_op_locations.h"
#include "framework/OpenWifiTypes.h"
#include "framework/MicroService.h"

#include "RESTObjects/RESTAPI_SecurityObjects.h"

namespace OpenWifi {

    static  ORM::FieldVec    OpLocationDB_Fields{
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
            ORM::Field{"phones",ORM::FieldType::FT_TEXT},
            ORM::Field{"mobiles",ORM::FieldType::FT_TEXT},
            ORM::Field{"geoCode",ORM::FieldType::FT_TEXT},
            ORM::Field{"operatorId",ORM::FieldType::FT_TEXT},
            ORM::Field{"tags",ORM::FieldType::FT_TEXT},
            ORM::Field{"managementPolicy",ORM::FieldType::FT_TEXT},
            ORM::Field{"subscriberId",ORM::FieldType::FT_TEXT}
    };

    static  ORM::IndexVec    OpLocationDB_Indexes{
            { std::string("op_location_name_index"),
              ORM::IndexEntryVec{
                      {std::string("name"),
                       ORM::Indextype::ASC} } }
    };

    OpLocationDB::OpLocationDB( OpenWifi::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L) :
            DB(T, "op_locations", OpLocationDB_Fields, OpLocationDB_Indexes, P, L, "olc") {}
}

template<> void ORM::DB<OpenWifi::OpLocationDBRecordType , OpenWifi::ProvObjects::OperatorLocation>::Convert(const OpenWifi::OpLocationDBRecordType &In, OpenWifi::ProvObjects::OperatorLocation &Out) {
    Out.info.id = In.get<0>();
    Out.info.name = In.get<1>();
    Out.info.description = In.get<2>();
    Out.info.notes = OpenWifi::RESTAPI_utils::to_object_array<OpenWifi::SecurityObjects::NoteInfo>(In.get<3>());
    Out.info.created = In.get<4>();
    Out.info.modified = In.get<5>();
    Out.type = In.get<6>();
    Out.buildingName = In.get<7>();
    Out.addressLines = OpenWifi::RESTAPI_utils::to_object_array(In.get<8>());
    Out.city = In.get<9>();
    Out.state = In.get<10>();
    Out.postal = In.get<11>();
    Out.country = In.get<12>();
    Out.phones = OpenWifi::RESTAPI_utils::to_object_array(In.get<13>());
    Out.mobiles = OpenWifi::RESTAPI_utils::to_object_array(In.get<14>());
    Out.geoCode = In.get<15>();
    Out.operatorId = In.get<16>();
    Out.info.tags = OpenWifi::RESTAPI_utils::to_taglist(In.get<17>());
    Out.managementPolicy = In.get<18>();
    Out.subscriberId = In.get<19>();
}

template<> void ORM::DB<OpenWifi::OpLocationDBRecordType, OpenWifi::ProvObjects::OperatorLocation>::Convert(const OpenWifi::ProvObjects::OperatorLocation &In, OpenWifi::OpLocationDBRecordType &Out) {
    Out.set<0>(In.info.id);
    Out.set<1>(In.info.name);
    Out.set<2>(In.info.description);
    Out.set<3>(OpenWifi::RESTAPI_utils::to_string(In.info.notes));
    Out.set<4>(In.info.created);
    Out.set<5>(In.info.modified);
    Out.set<6>(In.type);
    Out.set<7>(In.buildingName);
    Out.set<8>(OpenWifi::RESTAPI_utils::to_string(In.addressLines));
    Out.set<9>(In.city);
    Out.set<10>(In.state);
    Out.set<11>(In.postal);
    Out.set<12>(In.country);
    Out.set<13>(OpenWifi::RESTAPI_utils::to_string(In.phones));
    Out.set<14>(OpenWifi::RESTAPI_utils::to_string(In.mobiles));
    Out.set<15>(In.geoCode);
    Out.set<16>(In.operatorId);
    Out.set<17>(OpenWifi::RESTAPI_utils::to_string(In.info.tags));
    Out.set<18>(In.managementPolicy);
    Out.set<19>(In.subscriberId);
}
