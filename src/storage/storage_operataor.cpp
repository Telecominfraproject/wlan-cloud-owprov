//
// Created by stephane bourque on 2022-04-05.
//

#include "storage_operataor.h"
#include "framework/OpenWifiTypes.h"
#include "RESTObjects/RESTAPI_SecurityObjects.h"
#include "StorageService.h"
#include "framework/MicroService.h"

namespace OpenWifi {

    static  ORM::FieldVec    OperatorDB_Fields{
            // object info
            ORM::Field{"id",64, true},
            ORM::Field{"name",ORM::FieldType::FT_TEXT},
            ORM::Field{"description",ORM::FieldType::FT_TEXT},
            ORM::Field{"notes",ORM::FieldType::FT_TEXT},
            ORM::Field{"created",ORM::FieldType::FT_BIGINT},
            ORM::Field{"modified",ORM::FieldType::FT_BIGINT},
            ORM::Field{"contacts",ORM::FieldType::FT_TEXT},
            ORM::Field{"locations",ORM::FieldType::FT_TEXT},
            ORM::Field{"managementPolicy",ORM::FieldType::FT_TEXT},
            ORM::Field{"managementRoles",ORM::FieldType::FT_TEXT},
            ORM::Field{"rrm",ORM::FieldType::FT_TEXT},
            ORM::Field{"firmwareUpgrade",ORM::FieldType::FT_TEXT},
            ORM::Field{"firmwareRCOnly",ORM::FieldType::FT_BOOLEAN},
            ORM::Field{"variables",ORM::FieldType::FT_TEXT},
            ORM::Field{"defaultOperator",ORM::FieldType::FT_BOOLEAN},
            ORM::Field{"sourceIP",ORM::FieldType::FT_TEXT}
    };

    static  ORM::IndexVec    OperatorDB_Indexes{
            { std::string("operator_name_index"),
              ORM::IndexEntryVec{
                      {std::string("name"),
                       ORM::Indextype::ASC} } }
    };

    OperatorDB::OperatorDB( OpenWifi::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L) :
            DB(T, "operators", OperatorDB_Fields, OperatorDB_Indexes, P, L, "opr") {
    }

    bool OperatorDB::Upgrade([[maybe_unused]] uint32_t from, uint32_t &to) {
        to = Version();
        std::vector<std::string>    Script{

        };

        RunScript(Script);
        return true;
    }

    bool OperatorDB::GetByIP(const std::string &IP, std::string & uuid) {
        try {
            std::string UUID;
            std::function<bool(const ProvObjects::Operator &E)> Function = [&UUID,IP] (const ProvObjects::Operator &E) ->bool {
                if(E.sourceIP.empty())
                    return true;
                if(CIDR::IpInRanges(IP, E.sourceIP)) {
                    UUID = E.info.id;
                    return false;
                }
                return true;
            };
            Iterate(Function);
            uuid=UUID;
        } catch (const Poco::Exception &E) {
            Logger().log(E);
        }
        return false;
    }
}

template<> void ORM::DB<    OpenWifi::OperatorDBRecordType, OpenWifi::ProvObjects::Operator>::Convert(const OpenWifi::OperatorDBRecordType &In, OpenWifi::ProvObjects::Operator &Out) {
    Out.info.id = In.get<0>();
    Out.info.name = In.get<1>();
    Out.info.description = In.get<2>();
    Out.info.notes = OpenWifi::RESTAPI_utils::to_object_array<OpenWifi::SecurityObjects::NoteInfo>(In.get<3>());
    Out.info.created = In.get<4>();
    Out.info.modified = In.get<5>();
    Out.contacts = OpenWifi::RESTAPI_utils::to_object_array(In.get<6>());
    Out.locations = OpenWifi::RESTAPI_utils::to_object_array(In.get<7>());
    Out.managementPolicy = In.get<8>();
    Out.managementRoles = OpenWifi::RESTAPI_utils::to_object_array(In.get<9>());
    Out.rrm = In.get<10>();
    Out.firmwareUpgrade = In.get<11>();
    Out.firmwareRCOnly = In.get<12>();
    Out.variables = OpenWifi::RESTAPI_utils::to_object_array<OpenWifi::ProvObjects::Variable>(In.get<13>());
    Out.defaultOperator = In.get<14>();
    Out.sourceIP = OpenWifi::RESTAPI_utils::to_object_array(In.get<15>());
}

template<> void ORM::DB<    OpenWifi::OperatorDBRecordType, OpenWifi::ProvObjects::Operator>::Convert(const OpenWifi::ProvObjects::Operator &In, OpenWifi::OperatorDBRecordType &Out) {
    Out.set<0>(In.info.id);
    Out.set<1>(In.info.name);
    Out.set<2>(In.info.description);
    Out.set<3>(OpenWifi::RESTAPI_utils::to_string(In.info.notes));
    Out.set<4>(In.info.created);
    Out.set<5>(In.info.modified);
    Out.set<6>(OpenWifi::RESTAPI_utils::to_string(In.contacts));
    Out.set<7>(OpenWifi::RESTAPI_utils::to_string(In.locations));
    Out.set<8>(In.managementPolicy);
    Out.set<9>(OpenWifi::RESTAPI_utils::to_string(In.managementRoles));
    Out.set<10>(In.rrm);
    Out.set<11>(In.firmwareUpgrade);
    Out.set<12>(In.firmwareRCOnly);
    Out.set<13>(OpenWifi::RESTAPI_utils::to_string(In.variables));
    Out.set<14>(In.defaultOperator);
    Out.set<15>(OpenWifi::RESTAPI_utils::to_string(In.sourceIP));
}
