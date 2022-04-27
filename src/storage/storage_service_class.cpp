//
// Created by stephane bourque on 2022-04-06.
//

#include "storage_service_class.h"

//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "storage_entity.h"
#include "framework/OpenWifiTypes.h"
#include "RESTObjects/RESTAPI_SecurityObjects.h"
#include "StorageService.h"
#include "framework/MicroService.h"

namespace OpenWifi {
    static  ORM::FieldVec    ServiceClassDB_Fields{
            // object info
            ORM::Field{"id",64, true},
            ORM::Field{"name",ORM::FieldType::FT_TEXT},
            ORM::Field{"description",ORM::FieldType::FT_TEXT},
            ORM::Field{"notes",ORM::FieldType::FT_TEXT},
            ORM::Field{"created",ORM::FieldType::FT_BIGINT},
            ORM::Field{"modified",ORM::FieldType::FT_BIGINT},

            ORM::Field{"operatorId",ORM::FieldType::FT_TEXT},
            ORM::Field{"managementPolicy",ORM::FieldType::FT_TEXT},
            ORM::Field{"cost",ORM::FieldType::FT_REAL},
            ORM::Field{"currency",ORM::FieldType::FT_TEXT},
            ORM::Field{"period",ORM::FieldType::FT_TEXT},
            ORM::Field{"billingCode",ORM::FieldType::FT_TEXT},
            ORM::Field{"variables",ORM::FieldType::FT_TEXT},
            ORM::Field{"defaultService",ORM::FieldType::FT_BOOLEAN}
    };

    static  ORM::IndexVec    ServiceClassDB_Indexes{
            { std::string("service_class_name_index"),
              ORM::IndexEntryVec{
                      {std::string("name"),
                       ORM::Indextype::ASC} } }
    };

    ServiceClassDB::ServiceClassDB( OpenWifi::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L) :
            DB(T, "service_classes", ServiceClassDB_Fields, ServiceClassDB_Indexes, P, L, "scl") {
    }

    bool ServiceClassDB::Upgrade([[maybe_unused]] uint32_t from, uint32_t &to) {
        to = Version();
        std::vector<std::string>    Script{
        };
        RunScript(Script);
        return true;
    }

    std::string ServiceClassDB::DefaultForOperator(const std::string & OperatorId) {
        try {
            std::vector<ProvObjects::ServiceClass>  SC;
            StorageService()->ServiceClassDB().GetRecords(0,1,SC,fmt::format(" operatorId='{}' and defaultService=true ", OperatorId));
            if(SC.size()==1)
                return SC[0].info.id;
        } catch (...) {

        }
        return "";
    }

}

template<> void ORM::DB<    OpenWifi::ServiceClassDBRecordType, OpenWifi::ProvObjects::ServiceClass>::Convert(const OpenWifi::ServiceClassDBRecordType &In, OpenWifi::ProvObjects::ServiceClass &Out) {
    Out.info.id = In.get<0>();
    Out.info.name = In.get<1>();
    Out.info.description = In.get<2>();
    Out.info.notes = OpenWifi::RESTAPI_utils::to_object_array<OpenWifi::SecurityObjects::NoteInfo>(In.get<3>());
    Out.info.created = In.get<4>();
    Out.info.modified = In.get<5>();
    Out.operatorId = In.get<6>();
    Out.managementPolicy = In.get<7>();
    Out.cost = In.get<8>();
    Out.currency = In.get<9>();
    Out.period = In.get<10>();
    Out.billingCode = In.get<11>();
    Out.variables = OpenWifi::RESTAPI_utils::to_object_array<OpenWifi::ProvObjects::Variable>(In.get<12>());
    Out.defaultService = In.get<13>();
}

template<> void ORM::DB<    OpenWifi::ServiceClassDBRecordType, OpenWifi::ProvObjects::ServiceClass>::Convert(const OpenWifi::ProvObjects::ServiceClass &In, OpenWifi::ServiceClassDBRecordType &Out) {
    Out.set<0>(In.info.id);
    Out.set<1>(In.info.name);
    Out.set<2>(In.info.description);
    Out.set<3>(OpenWifi::RESTAPI_utils::to_string(In.info.notes));
    Out.set<4>(In.info.created);
    Out.set<5>(In.info.modified);
    Out.set<6>(In.operatorId);
    Out.set<7>(In.managementPolicy);
    Out.set<8>(In.cost);
    Out.set<9>(In.currency);
    Out.set<10>(In.period);
    Out.set<11>(In.billingCode);
    Out.set<12>(OpenWifi::RESTAPI_utils::to_string(In.variables));
    Out.set<13>(In.defaultService);
}
