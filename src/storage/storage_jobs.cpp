//
// Created by stephane bourque on 2021-10-28.
//

#include "storage_jobs.h"
#include "framework/OpenWifiTypes.h"
#include "framework/MicroService.h"

namespace OpenWifi {

    static  ORM::FieldVec    JobDB_Fields{
        // object info
        ORM::Field{"id",64, true},
        ORM::Field{"name",ORM::FieldType::FT_TEXT},
        ORM::Field{"description",ORM::FieldType::FT_TEXT},
        ORM::Field{"type",ORM::FieldType::FT_TEXT},
        ORM::Field{"progress",ORM::FieldType::FT_BIGINT},
        ORM::Field{"total",ORM::FieldType::FT_BIGINT},
        ORM::Field{"parameters",ORM::FieldType::FT_TEXT}
    };

    static  ORM::IndexVec    JobDB_Indexes{
        { std::string("job_name_index"),
          ORM::IndexEntryVec{
            {std::string("name"),
             ORM::Indextype::ASC} } }
    };

    JobDB::JobDB( OpenWifi::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L) :
    DB(T, "jobs", JobDB_Fields, JobDB_Indexes, P, L, "job") {}
}

template<> void ORM::DB<OpenWifi::JobDBRecordType, OpenWifi::JobRecord>::Convert(const OpenWifi::JobDBRecordType &In, OpenWifi::JobRecord &Out) {
    Out.id = In.get<0>();
    Out.name = In.get<1>();
    Out.description = In.get<2>();
    Out.type = In.get<3>();
    Out.progress = In.get<4>();
    Out.total = In.get<5>();
    Out.parameters = OpenWifi::RESTAPI_utils::to_array_of_array_of_object<OpenWifi::Job::Parameter>(In.get<3>());
}

template<> void ORM::DB<OpenWifi::JobDBRecordType, OpenWifi::JobRecord>::Convert(const OpenWifi::JobRecord &In, OpenWifi::JobDBRecordType &Out) {
    Out.set<0>(In.id);
    Out.set<1>(In.name);
    Out.set<2>(In.description);
    Out.set<3>(In.type);
    Out.set<4>(In.progress);
    Out.set<5>(In.total);
    Out.set<6>(OpenWifi::RESTAPI_utils::to_string(In.parameters));
}
