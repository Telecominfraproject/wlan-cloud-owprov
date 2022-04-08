//
// Created by stephane bourque on 2022-04-07.
//

#include "storage_op_contacts.h"
#include "framework/OpenWifiTypes.h"
#include "framework/MicroService.h"

#include "RESTObjects/RESTAPI_SecurityObjects.h"

namespace OpenWifi {

    static  ORM::FieldVec    OpContactDB_Fields{
            // object info
            ORM::Field{"id",64, true},
            ORM::Field{"name",ORM::FieldType::FT_TEXT},
            ORM::Field{"description",ORM::FieldType::FT_TEXT},
            ORM::Field{"notes",ORM::FieldType::FT_TEXT},
            ORM::Field{"created",ORM::FieldType::FT_BIGINT},
            ORM::Field{"modified",ORM::FieldType::FT_BIGINT},
            ORM::Field{"type",ORM::FieldType::FT_TEXT},
            ORM::Field{"title",ORM::FieldType::FT_TEXT},
            ORM::Field{"salutation",ORM::FieldType::FT_TEXT},
            ORM::Field{"firstname",ORM::FieldType::FT_TEXT},
            ORM::Field{"lastname",ORM::FieldType::FT_TEXT},
            ORM::Field{"initials",ORM::FieldType::FT_TEXT},
            ORM::Field{"visual",ORM::FieldType::FT_TEXT},
            ORM::Field{"mobiles",ORM::FieldType::FT_TEXT},
            ORM::Field{"phones",ORM::FieldType::FT_TEXT},
            ORM::Field{"primaryEmail",ORM::FieldType::FT_TEXT},
            ORM::Field{"secondaryEmail",ORM::FieldType::FT_TEXT},
            ORM::Field{"accessPIN",ORM::FieldType::FT_TEXT},
            ORM::Field{"operatorId",ORM::FieldType::FT_TEXT},
            ORM::Field{"tags",ORM::FieldType::FT_TEXT},
            ORM::Field{"managementPolicy",ORM::FieldType::FT_TEXT},
            ORM::Field{"subscriberDeviceId",ORM::FieldType::FT_TEXT}
    };

    static  ORM::IndexVec    OpContactDB_Indexes{
            { std::string("op_contact_name_index"),
              ORM::IndexEntryVec{
                      {std::string("name"),
                       ORM::Indextype::ASC} } }
    };

    OpContactDB::OpContactDB( OpenWifi::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L) :
            DB(T, "op_contacts", OpContactDB_Fields, OpContactDB_Indexes, P, L, "ocn") {}

}

template<> void ORM::DB<OpenWifi::OpContactDBRecordType, OpenWifi::ProvObjects::OperatorContact>::Convert(const OpenWifi::OpContactDBRecordType &In, OpenWifi::ProvObjects::OperatorContact &Out) {
    Out.info.id = In.get<0>();
    Out.info.name = In.get<1>();
    Out.info.description = In.get<2>();
    Out.info.notes = OpenWifi::RESTAPI_utils::to_object_array<OpenWifi::SecurityObjects::NoteInfo>(In.get<3>());
    Out.info.created = In.get<4>();
    Out.info.modified = In.get<5>();
    Out.type = In.get<6>();
    Out.title = In.get<7>();
    Out.salutation = In.get<8>();
    Out.firstname = In.get<9>();
    Out.lastname = In.get<10>();
    Out.initials = In.get<11>();
    Out.visual = In.get<12>();
    Out.mobiles = OpenWifi::RESTAPI_utils::to_object_array(In.get<13>());
    Out.phones = OpenWifi::RESTAPI_utils::to_object_array(In.get<14>());
    Out.primaryEmail = In.get<15>();
    Out.secondaryEmail = In.get<16>();
    Out.accessPIN = In.get<17>();
    Out.operatorId = In.get<18>();
    Out.info.tags = OpenWifi::RESTAPI_utils::to_taglist(In.get<19>());
    Out.managementPolicy = In.get<20>();
    Out.subscriberDeviceId = In.get<21>();
}

template<> void ORM::DB<OpenWifi::OpContactDBRecordType, OpenWifi::ProvObjects::OperatorContact>::Convert(const OpenWifi::ProvObjects::OperatorContact &In, OpenWifi::OpContactDBRecordType &Out) {
    Out.set<0>(In.info.id);
    Out.set<1>(In.info.name);
    Out.set<2>(In.info.description);
    Out.set<3>(OpenWifi::RESTAPI_utils::to_string(In.info.notes));
    Out.set<4>(In.info.created);
    Out.set<5>(In.info.modified);
    Out.set<6>(In.type);
    Out.set<7>(In.title);
    Out.set<8>(In.salutation);
    Out.set<9>(In.firstname);
    Out.set<10>(In.lastname);
    Out.set<11>(In.initials);
    Out.set<12>(In.visual);
    Out.set<13>(OpenWifi::RESTAPI_utils::to_string(In.mobiles));
    Out.set<14>(OpenWifi::RESTAPI_utils::to_string(In.phones));
    Out.set<15>(In.primaryEmail);
    Out.set<16>(In.secondaryEmail);
    Out.set<17>(In.accessPIN);
    Out.set<18>(In.operatorId);
    Out.set<19>(OpenWifi::RESTAPI_utils::to_string(In.info.tags));
    Out.set<20>(In.managementPolicy);
    Out.set<21>(In.subscriberDeviceId);
}
