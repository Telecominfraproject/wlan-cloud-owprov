//
// Created by stephane bourque on 2023-09-11.
//

#include "storage_glblrcerts.h"

#include <framework/orm.h>
#include "framework/OpenWifiTypes.h"
#include "framework/RESTAPI_utils.h"

#include "RESTObjects/RESTAPI_SecurityObjects.h"

namespace OpenWifi {

    static ORM::FieldVec GLBLRCertsDB_Fields{// object info
            ORM::Field{"id", 64, true},
            ORM::Field{"name", ORM::FieldType::FT_TEXT},
            ORM::Field{"accountId", ORM::FieldType::FT_TEXT},
            ORM::Field{"csr", ORM::FieldType::FT_TEXT},
            ORM::Field{"certificate", ORM::FieldType::FT_TEXT},
            ORM::Field{"certificateChain", ORM::FieldType::FT_TEXT},
            ORM::Field{"certificateId", ORM::FieldType::FT_TEXT},
            ORM::Field{"expiresAt", ORM::FieldType::FT_BIGINT},
            ORM::Field{"created", ORM::FieldType::FT_BIGINT}
    };

    static ORM::IndexVec GLBLRCertsDB_Indexes{
            {std::string("glblr_cert_id_index"),
             ORM::IndexEntryVec{{std::string("name"), ORM::Indextype::ASC}}}};

    GLBLRCertsDB::GLBLRCertsDB(OpenWifi::DBType T, Poco::Data::SessionPool &P, Poco::Logger &L)
            : DB(T, "glblr_certs", GLBLRCertsDB_Fields, GLBLRCertsDB_Indexes, P, L, "glc") {}

    bool GLBLRCertsDB::Upgrade([[maybe_unused]] uint32_t from, uint32_t &to) {
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
void ORM::DB<OpenWifi::GLBLRCertsDBRecordType, OpenWifi::ProvObjects::GLBLRCertificateInfo>::Convert(
        const OpenWifi::GLBLRCertsDBRecordType &In, OpenWifi::ProvObjects::GLBLRCertificateInfo &Out) {
    Out.id = In.get<0>();
    Out.name = In.get<1>();
    Out.accountId = In.get<2>();
    Out.csr = In.get<3>();
    Out.certificate = In.get<4>();
    Out.certificateChain = OpenWifi::RESTAPI_utils::to_object_array(In.get<5>());
    Out.certificateId = In.get<6>();
    Out.expiresAt = In.get<7>();
    Out.created = In.get<8>();
}

template <>
void ORM::DB<OpenWifi::GLBLRCertsDBRecordType, OpenWifi::ProvObjects::GLBLRCertificateInfo>::Convert(
        const OpenWifi::ProvObjects::GLBLRCertificateInfo &In, OpenWifi::GLBLRCertsDBRecordType &Out) {
    Out.set<0>(In.id);
    Out.set<1>(In.name);
    Out.set<2>(In.accountId);
    Out.set<3>(In.csr);
    Out.set<4>(In.certificate);
    Out.set<5>(OpenWifi::RESTAPI_utils::to_string(In.certificateChain));
    Out.set<6>(In.certificateId);
    Out.set<7>(In.expiresAt);
    Out.set<8>(In.created);
}
