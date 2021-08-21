//
// Created by stephane bourque on 2021-08-20.
//

#include "storage_management_roles.h"
#include "uCentralTypes.h"
#include "RESTAPI_utils.h"
#include "RESTAPI_SecurityObjects.h"

namespace OpenWifi {

    static  ORM::FieldVec    RolesDB_Fields{
        // object info
        ORM::Field{"id",64, true},
        ORM::Field{"name",ORM::FieldType::FT_TEXT},
        ORM::Field{"description",ORM::FieldType::FT_TEXT},
        ORM::Field{"notes",ORM::FieldType::FT_TEXT},
        ORM::Field{"created",ORM::FieldType::FT_BIGINT},
        ORM::Field{"modified",ORM::FieldType::FT_BIGINT},
        ORM::Field{"managementPolicy",ORM::FieldType::FT_TEXT},
        ORM::Field{"users",ORM::FieldType::FT_TEXT}
    };

    static  ORM::IndexVec    RolesDB_Indexes{
        { std::string("roles_name_index"),
          ORM::IndexEntryVec{
            {std::string("name"),
             ORM::Indextype::ASC} } }
    };

    ManagementRoleDB::ManagementRoleDB( ORM::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L) :
        DB(T, "roles", RolesDB_Fields, RolesDB_Indexes, P, L) {}

}

template<> void ORM::DB<    OpenWifi::ManagementRoleDBRecordType, OpenWifi::ProvObjects::ManagementRole>::Convert(OpenWifi::ManagementRoleDBRecordType &In, OpenWifi::ProvObjects::ManagementRole &Out) {
    Out.info.id = In.get<0>();
    Out.info.name = In.get<1>();
    Out.info.description = In.get<2>();
    Out.info.notes = uCentral::RESTAPI_utils::to_object_array<uCentral::SecurityObjects::NoteInfo>(In.get<3>());
    Out.info.created = In.get<4>();
    Out.info.modified = In.get<5>();
    Out.managementPolicy = In.get<6>();
    Out.users = uCentral::RESTAPI_utils::to_object_array<OpenWifi::ProvObjects::UserInfoDigest>(In.get<7>());
}

template<> void ORM::DB<    OpenWifi::ManagementRoleDBRecordType, OpenWifi::ProvObjects::ManagementRole>::Convert(OpenWifi::ProvObjects::ManagementRole &In, OpenWifi::ManagementRoleDBRecordType &Out) {
    Out.set<0>(In.info.id);
    Out.set<1>(In.info.name);
    Out.set<2>(In.info.description);
    Out.set<3>(uCentral::RESTAPI_utils::to_string(In.info.notes));
    Out.set<4>(In.info.created);
    Out.set<5>(In.info.modified);
    Out.set<6>(In.managementPolicy);
    Out.set<7>(uCentral::RESTAPI_utils::to_string(In.users));
}
