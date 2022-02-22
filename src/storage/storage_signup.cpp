//
// Created by stephane bourque on 2022-02-21.
//

#include "storage_signup.h"

#include "framework/OpenWifiTypes.h"
#include "RESTObjects/RESTAPI_SecurityObjects.h"
#include "StorageService.h"
#include "framework/MicroService.h"

namespace OpenWifi {

    const static  ORM::FieldVec    SignupDB_Fields{
            // object info
            ORM::Field{"id",64, true},
            ORM::Field{"name",ORM::FieldType::FT_TEXT},
            ORM::Field{"description",ORM::FieldType::FT_TEXT},
            ORM::Field{"notes",ORM::FieldType::FT_TEXT},
            ORM::Field{"created",ORM::FieldType::FT_BIGINT},
            ORM::Field{"modified",ORM::FieldType::FT_BIGINT},
            ORM::Field{"email",ORM::FieldType::FT_TEXT},
            ORM::Field{"userId",ORM::FieldType::FT_TEXT},
            ORM::Field{"serialNumber",ORM::FieldType::FT_TEXT},
            ORM::Field{"created",ORM::FieldType::FT_BIGINT},
            ORM::Field{"completed",ORM::FieldType::FT_BIGINT},
            ORM::Field{"status",ORM::FieldType::FT_TEXT}
    };

    const static  ORM::IndexVec    SignupDB_Indexes{
            { std::string("signup_email_index"),
              ORM::IndexEntryVec{
                      {std::string("email"),
                       ORM::Indextype::ASC} } }
    };

    SignupDB::SignupDB( OpenWifi::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L) :
            DB(T, "signups", SignupDB_Fields, SignupDB_Indexes, P, L, "sig") {
    }

}

template<> void ORM::DB<    OpenWifi::SignupDBRecordType, OpenWifi::ProvObjects::SignupEntry>::Convert(const OpenWifi::SignupDBRecordType &In, OpenWifi::ProvObjects::SignupEntry &Out) {
    Out.info.id = In.get<0>();
    Out.info.name = In.get<1>();
    Out.info.description = In.get<2>();
    Out.info.notes = OpenWifi::RESTAPI_utils::to_object_array<OpenWifi::SecurityObjects::NoteInfo>(In.get<3>());
    Out.info.created = In.get<4>();
    Out.info.modified = In.get<5>();
    Out.email = In.get<6>();
    Out.userId = In.get<7>();
    Out.serialNumber = In.get<8>();
    Out.created = In.get<9>();
    Out.completed = In.get<10>();
    Out.status = In.get<11>();
}

template<> void ORM::DB<    OpenWifi::SignupDBRecordType, OpenWifi::ProvObjects::SignupEntry>::Convert(const OpenWifi::ProvObjects::SignupEntry &In, OpenWifi::SignupDBRecordType &Out) {
    Out.set<0>(In.info.id);
    Out.set<1>(In.info.name);
    Out.set<2>(In.info.description);
    Out.set<3>(OpenWifi::RESTAPI_utils::to_string(In.info.notes));
    Out.set<4>(In.info.created);
    Out.set<5>(In.info.modified);
    Out.set<6>(In.email);
    Out.set<7>(In.userId);
    Out.set<8>(In.serialNumber);
    Out.set<9>(In.created);
    Out.set<10>(In.created);
    Out.set<11>(In.status);
}
