//
// Created by stephane bourque on 2023-09-17.
//

#include "storage_orion_accounts.h"
#include <framework/orm.h>
#include "framework/OpenWifiTypes.h"
#include "framework/RESTAPI_utils.h"

#include "RESTObjects/RESTAPI_SecurityObjects.h"

namespace OpenWifi {

    static ORM::FieldVec OrionAccountsDB_Fields{
            ORM::Field{"id", 64, true},
            ORM::Field{"name", ORM::FieldType::FT_TEXT},
            ORM::Field{"description", ORM::FieldType::FT_TEXT},
            ORM::Field{"notes", ORM::FieldType::FT_TEXT},
            ORM::Field{"created", ORM::FieldType::FT_BIGINT},
            ORM::Field{"modified", ORM::FieldType::FT_BIGINT},
            ORM::Field{"privateKey", ORM::FieldType::FT_TEXT},
            ORM::Field{"certificate", ORM::FieldType::FT_TEXT},
            ORM::Field{"cacerts", ORM::FieldType::FT_TEXT}
    };

    static ORM::IndexVec OrionAccountsDB_Indexes{
            {std::string("orion_name_index"),
             ORM::IndexEntryVec{{std::string("name"), ORM::Indextype::ASC}}}};

    OrionAccountsDB::OrionAccountsDB(OpenWifi::DBType T, Poco::Data::SessionPool &P, Poco::Logger &L)
            : DB(T, "orion_accts", OrionAccountsDB_Fields, OrionAccountsDB_Indexes, P, L, "oat") {}

    bool OrionAccountsDB::Upgrade([[maybe_unused]] uint32_t from, uint32_t &to) {
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
void ORM::DB<OpenWifi::OrionAccountsDBRecordType, OpenWifi::ProvObjects::GooglOrionAccountInfo>::Convert(
        const OpenWifi::OrionAccountsDBRecordType &In, OpenWifi::ProvObjects::GooglOrionAccountInfo &Out) {
    Out.info.id = In.get<0>();
    Out.info.name = In.get<1>();
    Out.info.description = In.get<2>();
    Out.info.notes =
            OpenWifi::RESTAPI_utils::to_object_array<OpenWifi::SecurityObjects::NoteInfo>(In.get<3>());
    Out.info.created = In.get<4>();
    Out.info.modified = In.get<5>();
    Out.privateKey =In.get<6>();
    Out.certificate = In.get<7>();
    Out.cacerts = OpenWifi::RESTAPI_utils::to_object_array(In.get<8>());
}

template <>
void ORM::DB<OpenWifi::OrionAccountsDBRecordType, OpenWifi::ProvObjects::GooglOrionAccountInfo>::Convert(
        const OpenWifi::ProvObjects::GooglOrionAccountInfo &In, OpenWifi::OrionAccountsDBRecordType &Out) {
    Out.set<0>(In.info.id);
    Out.set<1>(In.info.name);
    Out.set<2>(In.info.description);
    Out.set<3>(OpenWifi::RESTAPI_utils::to_string(In.info.notes));
    Out.set<4>(In.info.created);
    Out.set<5>(In.info.modified);
    Out.set<6>(In.privateKey);
    Out.set<7>(In.certificate);
    Out.set<8>(OpenWifi::RESTAPI_utils::to_string(In.cacerts));
}
