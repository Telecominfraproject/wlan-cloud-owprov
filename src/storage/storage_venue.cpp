//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include <functional>

#include "storage_venue.h"
#include "framework/MicroService.h"
#include "framework/OpenWifiTypes.h"
#include "RESTObjects/RESTAPI_SecurityObjects.h"
#include "StorageService.h"

namespace OpenWifi {

    static  ORM::FieldVec    VenueDB_Fields{
        // object info
        ORM::Field{"id",64, true},
        ORM::Field{"name",ORM::FieldType::FT_TEXT},
        ORM::Field{"description",ORM::FieldType::FT_TEXT},
        ORM::Field{"notes",ORM::FieldType::FT_TEXT},
        ORM::Field{"created",ORM::FieldType::FT_BIGINT},
        ORM::Field{"modified",ORM::FieldType::FT_BIGINT},
        ORM::Field{"entity",ORM::FieldType::FT_TEXT},
        ORM::Field{"parent",ORM::FieldType::FT_TEXT},
        ORM::Field{"children",ORM::FieldType::FT_TEXT},
        ORM::Field{"devices",ORM::FieldType::FT_TEXT},
        ORM::Field{"managementPolicy",ORM::FieldType::FT_TEXT},
        ORM::Field{"topology",ORM::FieldType::FT_TEXT},
        ORM::Field{"design",ORM::FieldType::FT_TEXT},
        ORM::Field{"contact",ORM::FieldType::FT_TEXT},
        ORM::Field{"location",ORM::FieldType::FT_TEXT},
        ORM::Field{"rrm",ORM::FieldType::FT_TEXT},
        ORM::Field{"tags",ORM::FieldType::FT_TEXT},
        ORM::Field{"deviceConfiguration",ORM::FieldType::FT_TEXT},
        ORM::Field{"sourceIP",ORM::FieldType::FT_TEXT}
    };

    static  ORM::IndexVec    VenueDB_Indexes{
        { std::string("venue_name_index"),
          ORM::IndexEntryVec{
            {std::string("name"),
             ORM::Indextype::ASC} } }
    };

    VenueDB::VenueDB( OpenWifi::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L) :
        DB(T, "venues", VenueDB_Fields, VenueDB_Indexes, P, L, "ven") {}

    bool VenueDB::CreateShortCut(ProvObjects::Venue &V) {
        if(StorageService()->VenueDB().CreateRecord(V)) {
            if(!V.parent.empty())
                StorageService()->VenueDB().AddChild("id", V.parent, V.info.id);
            if(!V.entity.empty())
                StorageService()->EntityDB().AddVenue("id", V.entity, V.info.id);
            if(!V.location.empty())
                StorageService()->LocationDB().AddInUse("id",V.location, StorageService()->VenueDB().Prefix(), V.info.id);
            if(!V.contact.empty())
                StorageService()->ContactDB().AddInUse("id",V.contact, StorageService()->VenueDB().Prefix(), V.info.id);
            if(!V.managementPolicy.empty())
                StorageService()->PolicyDB().AddInUse("id",V.managementPolicy, StorageService()->VenueDB().Prefix(), V.info.id);
            if(!V.deviceConfiguration.empty()) {
                for(auto &i:V.deviceConfiguration)
                    StorageService()->ConfigurationDB().AddInUse("id", i, StorageService()->ConfigurationDB().Prefix(), V.info.id);
            }
            ProvObjects::Venue  NV;
            StorageService()->VenueDB().GetRecord("id",V.info.id,NV);
            V = NV;
            return true;
        }
        return false;
    }

    bool VenueDB::GetByIP(const std::string &IP, std::string & uuid) {
        try {
            std::string UUID;
            std::function<bool(const ProvObjects::Venue &E)> Function = [&UUID,IP] (const ProvObjects::Venue &E) ->bool {
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

template<> void ORM::DB<    OpenWifi::VenueDBRecordType, OpenWifi::ProvObjects::Venue>::Convert(OpenWifi::VenueDBRecordType &In, OpenWifi::ProvObjects::Venue &Out) {
    Out.info.id = In.get<0>();
    Out.info.name = In.get<1>();
    Out.info.description = In.get<2>();
    Out.info.notes = OpenWifi::RESTAPI_utils::to_object_array<OpenWifi::SecurityObjects::NoteInfo>(In.get<3>());
    Out.info.created = In.get<4>();
    Out.info.modified = In.get<5>();
    Out.entity = In.get<6>();
    Out.parent = In.get<7>();
    OpenWifi::Types::from_string(In.get<8>(), Out.children);
    OpenWifi::Types::from_string(In.get<9>(), Out.devices);
    Out.managementPolicy = In.get<10>();
    Out.topology = OpenWifi::RESTAPI_utils::to_object_array<OpenWifi::ProvObjects::DiGraphEntry>(In.get<11>());
    Out.design = In.get<12>();
    Out.contact = In.get<13>();
    Out.location = In.get<14>();
    Out.rrm = In.get<15>();
    Out.info.tags = OpenWifi::RESTAPI_utils::to_taglist(In.get<16>());
    OpenWifi::Types::from_string(In.get<17>(), Out.deviceConfiguration);
    OpenWifi::Types::from_string(In.get<18>(), Out.sourceIP);

}

template<> void ORM::DB<    OpenWifi::VenueDBRecordType, OpenWifi::ProvObjects::Venue>::Convert(OpenWifi::ProvObjects::Venue &In, OpenWifi::VenueDBRecordType &Out) {
    Out.set<0>(In.info.id);
    Out.set<1>(In.info.name);
    Out.set<2>(In.info.description);
    Out.set<3>(OpenWifi::RESTAPI_utils::to_string(In.info.notes));
    Out.set<4>(In.info.created);
    Out.set<5>(In.info.modified);
    Out.set<6>(In.entity);
    Out.set<7>(In.parent);
    Out.set<8>(OpenWifi::Types::to_string(In.children));
    Out.set<9>(OpenWifi::Types::to_string(In.devices));
    Out.set<10>(In.managementPolicy);
    Out.set<11>(OpenWifi::RESTAPI_utils::to_string(In.topology));
    Out.set<12>(In.design);
    Out.set<13>(In.contact);
    Out.set<14>(In.location);
    Out.set<15>(In.rrm);
    Out.set<16>(OpenWifi::RESTAPI_utils::to_string(In.info.tags));
    Out.set<17>(OpenWifi::Types::to_string(In.deviceConfiguration));
    Out.set<18>(OpenWifi::Types::to_string(In.sourceIP));
}