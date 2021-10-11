//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//


#include "storage_contact.h"
#include "Utils.h"
#include "OpenWifiTypes.h"
#include "RESTAPI_utils.h"
#include "RESTAPI_SecurityObjects.h"

namespace OpenWifi {

    static  ORM::FieldVec    ContactDB_Fields{
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
        ORM::Field{"inUse",ORM::FieldType::FT_TEXT},
        ORM::Field{"tags",ORM::FieldType::FT_TEXT},
        ORM::Field{"managementPolicy",ORM::FieldType::FT_TEXT},
        ORM::Field{"owner",ORM::FieldType::FT_TEXT}
    };

    static  ORM::IndexVec    ContactDB_Indexes{
        { std::string("contact_name_index"),
          ORM::IndexEntryVec{
            {std::string("name"),
             ORM::Indextype::ASC} } }
    };

    ContactDB::ContactDB( OpenWifi::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L) :
        DB(T, "contacts", ContactDB_Fields, ContactDB_Indexes, P, L, "con") {}

}

template<> void ORM::DB<OpenWifi::ContactDBRecordType, OpenWifi::ProvObjects::Contact>::Convert(OpenWifi::ContactDBRecordType &In, OpenWifi::ProvObjects::Contact &Out) {
    Out.info.id = In.get<0>();
    Out.info.name = In.get<1>();
    Out.info.description = In.get<2>();
    Out.info.notes = OpenWifi::RESTAPI_utils::to_object_array<OpenWifi::SecurityObjects::NoteInfo>(In.get<3>());
    Out.info.created = In.get<4>();
    Out.info.modified = In.get<5>();
    Out.type = OpenWifi::ProvObjects::contact_from_string(In.get<6>());
    Out.title = In.get<7>();
    Out.salutation = In.get<8>();
    Out.firstname = In.get<9>();
    Out.lastname = In.get<10>();
    Out.initials = In.get<11>();
    Out.visual = In.get<12>();
    OpenWifi::Types::from_string(In.get<13>(), Out.mobiles);
    OpenWifi::Types::from_string(In.get<14>(), Out.phones);
    Out.primaryEmail = In.get<15>();
    Out.secondaryEmail = In.get<16>();
    Out.accessPIN = In.get<17>();
    OpenWifi::Types::from_string(In.get<18>(), Out.inUse);
    Out.info.tags = OpenWifi::RESTAPI_utils::to_taglist(In.get<19>());
    Out.managementPolicy = In.get<20>();
    Out.owner = In.get<21>();
}

template<> void ORM::DB<OpenWifi::ContactDBRecordType, OpenWifi::ProvObjects::Contact>::Convert(OpenWifi::ProvObjects::Contact &In, OpenWifi::ContactDBRecordType &Out) {
    Out.set<0>(In.info.id);
    Out.set<1>(In.info.name);
    Out.set<2>(In.info.description);
    Out.set<3>(OpenWifi::RESTAPI_utils::to_string(In.info.notes));
    Out.set<4>(In.info.created);
    Out.set<5>(In.info.modified);
    Out.set<6>(to_string(In.type));
    Out.set<7>(In.title);
    Out.set<8>(In.salutation);
    Out.set<9>(In.firstname);
    Out.set<10>(In.lastname);
    Out.set<11>(In.initials);
    Out.set<12>(In.visual);
    Out.set<13>(OpenWifi::Types::to_string(In.mobiles));
    Out.set<14>(OpenWifi::Types::to_string(In.phones));
    Out.set<15>(In.primaryEmail);
    Out.set<16>(In.secondaryEmail);
    Out.set<17>(In.accessPIN);
    Out.set<18>(OpenWifi::Types::to_string(In.inUse));
    Out.set<19>(OpenWifi::RESTAPI_utils::to_string(In.info.tags));
    Out.set<20>(In.managementPolicy);
    Out.set<21>(In.owner);
}
