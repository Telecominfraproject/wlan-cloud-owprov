//
// Created by stephane bourque on 2023-09-11.
//

#include "storage_glblraccounts.h"
#include <framework/orm.h>
#include "framework/OpenWifiTypes.h"
#include "framework/RESTAPI_utils.h"

#include "RESTObjects/RESTAPI_SecurityObjects.h"

namespace OpenWifi {

    static ORM::FieldVec GLBLRAccountInfoDB_Fields{// object info
            ORM::Field{"id", 64, true},
            ORM::Field{"name", ORM::FieldType::FT_TEXT},
            ORM::Field{"description", ORM::FieldType::FT_TEXT},
            ORM::Field{"notes", ORM::FieldType::FT_TEXT},
            ORM::Field{"created", ORM::FieldType::FT_BIGINT},
            ORM::Field{"modified", ORM::FieldType::FT_BIGINT},
            ORM::Field{"privateKey", ORM::FieldType::FT_TEXT},
            ORM::Field{"country", ORM::FieldType::FT_TEXT},
            ORM::Field{"province", ORM::FieldType::FT_TEXT},
            ORM::Field{"city", ORM::FieldType::FT_TEXT},
            ORM::Field{"organization", ORM::FieldType::FT_TEXT},
            ORM::Field{"commonName", ORM::FieldType::FT_TEXT}
    };

    static ORM::IndexVec GLBLRAccountInfoDB_Indexes{
            {std::string("glblr_name_index"),
             ORM::IndexEntryVec{{std::string("name"), ORM::Indextype::ASC}}}};

    GLBLRAccountInfoDB::GLBLRAccountInfoDB(OpenWifi::DBType T, Poco::Data::SessionPool &P, Poco::Logger &L)
            : DB(T, "glblr_accts", GLBLRAccountInfoDB_Fields, GLBLRAccountInfoDB_Indexes, P, L, "glr") {}

    bool GLBLRAccountInfoDB::Upgrade([[maybe_unused]] uint32_t from, uint32_t &to) {
        to = Version();
        std::vector<std::string> Script{};

        for (const auto &i : Script) {
            try {
                auto Session = Pool_.get();
                Session << i, Poco::Data::Keywords::now;
            } catch (...) {
            }
        }
        return true;
    }

} // namespace OpenWifi

template <>
void ORM::DB<OpenWifi::GLBLRAccountsDBRecordType, OpenWifi::ProvObjects::GLBLRAccountInfo>::Convert(
        const OpenWifi::GLBLRAccountsDBRecordType &In, OpenWifi::ProvObjects::GLBLRAccountInfo &Out) {
    Out.info.id = In.get<0>();
    Out.info.name = In.get<1>();
    Out.info.description = In.get<2>();
    Out.info.notes =
            OpenWifi::RESTAPI_utils::to_object_array<OpenWifi::SecurityObjects::NoteInfo>(In.get<3>());
    Out.info.created = In.get<4>();
    Out.info.modified = In.get<5>();
    Out.privateKey =In.get<6>();
    Out.country = In.get<7>();
    Out.province = In.get<8>();
    Out.city = In.get<9>();
    Out.organization = In.get<10>();
    Out.commonName = In.get<11>();
}

template <>
void ORM::DB<OpenWifi::GLBLRAccountsDBRecordType, OpenWifi::ProvObjects::GLBLRAccountInfo>::Convert(
        const OpenWifi::ProvObjects::GLBLRAccountInfo &In, OpenWifi::GLBLRAccountsDBRecordType &Out) {
    Out.set<0>(In.info.id);
    Out.set<1>(In.info.name);
    Out.set<2>(In.info.description);
    Out.set<3>(OpenWifi::RESTAPI_utils::to_string(In.info.notes));
    Out.set<4>(In.info.created);
    Out.set<5>(In.info.modified);
    Out.set<6>(In.privateKey);
    Out.set<7>(In.country);
    Out.set<8>(In.province);
    Out.set<9>(In.city);
    Out.set<10>(In.organization);
    Out.set<11>(In.commonName);
}
