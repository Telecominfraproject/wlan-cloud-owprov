//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//


#include "storage_inventory.h"
#include "framework/OpenWifiTypes.h"
#include "framework/MicroService.h"
#include "RESTObjects/RESTAPI_SecurityObjects.h"
#include "StorageService.h"
#include "sdks/SDK_gw.h"
#include "AutoDiscovery.h"
#include "SerialNumberCache.h"
#include "Daemon.h"

namespace OpenWifi {

    static  ORM::FieldVec    InventoryDB_Fields{
        // object info
        ORM::Field{"id",64, true},
        ORM::Field{"name",ORM::FieldType::FT_TEXT},
        ORM::Field{"description",ORM::FieldType::FT_TEXT},
        ORM::Field{"notes",ORM::FieldType::FT_TEXT},
        ORM::Field{"created",ORM::FieldType::FT_BIGINT},
        ORM::Field{"modified",ORM::FieldType::FT_BIGINT},
        ORM::Field{"serialNumber",ORM::FieldType::FT_TEXT},
        ORM::Field{"venue",ORM::FieldType::FT_TEXT},
        ORM::Field{"entity",ORM::FieldType::FT_TEXT},
        ORM::Field{"subscriber",ORM::FieldType::FT_TEXT},
        ORM::Field{"deviceType",ORM::FieldType::FT_TEXT},
        ORM::Field{"qrcode",ORM::FieldType::FT_TEXT},
        ORM::Field{"geocode",ORM::FieldType::FT_TEXT},
        ORM::Field{"location",ORM::FieldType::FT_TEXT},
        ORM::Field{"contact",ORM::FieldType::FT_TEXT},
        ORM::Field{"deviceConfiguration",ORM::FieldType::FT_TEXT},
        ORM::Field{"rrm",ORM::FieldType::FT_TEXT},
        ORM::Field{"tags",ORM::FieldType::FT_TEXT},
        ORM::Field{"managementPolicy",ORM::FieldType::FT_TEXT},
        ORM::Field{"state",ORM::FieldType::FT_TEXT},
        ORM::Field{"devClass",ORM::FieldType::FT_TEXT},
        ORM::Field{"locale",ORM::FieldType::FT_TEXT},
        ORM::Field{"realMacAddress",ORM::FieldType::FT_TEXT}
    };

    static  ORM::IndexVec    InventoryDB_Indexes{
        { std::string("inventory_name_index"),
          ORM::IndexEntryVec{
            {std::string("name"),
             ORM::Indextype::ASC} } },
         { std::string("inventory_serial_index"),
           ORM::IndexEntryVec{
            {std::string("serialNumber"),
             ORM::Indextype::ASC} } }
    };

    bool InventoryDB::Upgrade(uint32_t from, uint32_t &to) {
        to = Version();
        std::vector<std::string>    Script{
            "alter table " + TableName_ + " add column state text" ,
            "alter table " + TableName_ + " add column locale varchar(16)" ,
            "alter table " + TableName_ + " add column realMacAddress text" ,
            "alter table " + TableName_ + " add column devClass text"
        };

        for(const auto &i:Script) {
            try {
                auto Session = Pool_.get();
                Session << i , Poco::Data::Keywords::now;
            } catch (...) {

            }
        }
        return true;
    }

#define __DBG__ std::cout << __FILE__ << ": " << __LINE__ << std::endl;

    InventoryDB::InventoryDB( OpenWifi::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L) :
        DB(T, "inventory", InventoryDB_Fields, InventoryDB_Indexes, P, L, "inv") {}

    bool InventoryDB::CreateFromConnection( const std::string &SerialNumber,
                                            const std::string &ConnectionInfo,
                                            const std::string &DeviceType,
                                            const std::string &Locale) {

        ProvObjects::InventoryTag   ExistingDevice;
        if(!GetRecord("serialNumber",SerialNumber,ExistingDevice)) {
            ProvObjects::InventoryTag   NewDevice;
            uint64_t Now = std::time(nullptr);

            auto Tokens = Poco::StringTokenizer(ConnectionInfo,"@:");
            std::string IP;
            if(Tokens.count()==3) {
                IP = Tokens[1];
            }
            NewDevice.info.id = MicroService::CreateUUID();
            NewDevice.info.name = SerialNumber;
            NewDevice.info.created = NewDevice.info.modified = Now;
            NewDevice.info.notes.push_back(SecurityObjects::NoteInfo{.created=Now,.createdBy="*system",.note="Auto discovered"});
            NewDevice.serialNumber = SerialNumber;
            NewDevice.deviceType = DeviceType;
            NewDevice.locale = Locale;
            nlohmann::json StateDoc;
            StateDoc["method"] = "auto-discovery";
            StateDoc["date"] = std::time(nullptr);
            NewDevice.state = to_string(StateDoc);
            NewDevice.devClass = "any";
            if(!IP.empty()) {
                StorageService()->VenueDB().GetByIP(IP,NewDevice.venue);
                if(NewDevice.venue.empty()) {
                    StorageService()->EntityDB().GetByIP(IP,NewDevice.entity);
                }
            }

            if(CreateRecord(NewDevice)) {
                SerialNumberCache()->AddSerialNumber(SerialNumber, DeviceType);
                std::string FullUUID;
                if(!NewDevice.entity.empty()) {
                    StorageService()->EntityDB().AddDevice("id",NewDevice.entity,NewDevice.info.id);
                    FullUUID = StorageService()->EntityDB().Prefix() + ":" + NewDevice.entity;
                }
                else if(!NewDevice.venue.empty()) {
                    StorageService()->VenueDB().AddDevice("id",NewDevice.venue,NewDevice.info.id);
                    FullUUID = StorageService()->VenueDB().Prefix() + ":" + NewDevice.venue;
                }

                if(!FullUUID.empty()) {
                    if(SDK::GW::Device::SetVenue(nullptr,NewDevice.serialNumber,FullUUID)) {
                        // std::cout << "Set GW done " << SerialNumber << std::endl;
                        Logger().information(Poco::format("%s: GW set entity/venue property.", NewDevice.serialNumber));
                    } else {
                        // std::cout << "Could not set GW " << SerialNumber << std::endl;
                        Logger().information(Poco::format("%s: could not set GW entity/venue property.", NewDevice.serialNumber));
                    }
                }
                Logger().information(Poco::format("Adding %s to inventory.",SerialNumber));
                return true;
            } else {
                Logger().information(Poco::format("Could not add %s to inventory.",SerialNumber));
            }
        } else {
            //  Device already exists, do we need to modify anything?
            bool modified=false;
            if(ExistingDevice.deviceType != DeviceType) {
                ExistingDevice.deviceType = DeviceType;
                modified = true;
            }

            //  if this device is being claimed, not it is claimed.
            if(!ExistingDevice.state.empty()) {
                auto State = nlohmann::json::parse(ExistingDevice.state);
                if(State["method"] == "claiming") {
                    uint64_t Date = State["date"];
                    uint64_t Now = std::time(nullptr);

                    if((Now - Date)<(24*60*60)) {
                        State["method"] = "claimed";
                        State["date"] = std::time(nullptr);
                        ExistingDevice.state = to_string(State);
                        modified = true;
                    } else {
                        ExistingDevice.state = "";
                        modified = true;
                    }
                }
            } else {
                ExistingDevice.devClass = "any";
                modified = true;
            }

            if(Locale!=ExistingDevice.locale) {
                ExistingDevice.locale = Locale;
                modified = true;
            }

            if(modified) {
                ExistingDevice.info.modified = std::time(nullptr);
                StorageService()->InventoryDB().UpdateRecord("serialNumber", SerialNumber, ExistingDevice);
            }
        }
        return false;
    }

    bool InventoryDB::FindFirmwareOptionsForEntity(const std::string &EntityUUID, ProvObjects::FIRMWARE_UPGRADE_RULES & Rules) {
        std::string UUID = EntityUUID;
        while(!UUID.empty() && UUID!=EntityDB::RootUUID()) {
            ProvObjects::Entity                 E;
            if(StorageService()->EntityDB().GetRecord("id",UUID,E)) {
                if(!E.deviceConfiguration.empty()) {
                    ProvObjects::DeviceConfiguration    C;
                    for(const auto &i:E.deviceConfiguration) {
                        if(StorageService()->ConfigurationDB().GetRecord("id",i,C)) {
                            if(C.firmwareUpgrade=="no") {
                                Rules = ProvObjects::dont_upgrade;
                                return false;
                            }
                            if(C.firmwareUpgrade=="yes") {
                                if(C.firmwareRCOnly)
                                    Rules = ProvObjects::upgrade_release_only;
                                else
                                    Rules = ProvObjects::upgrade_latest;
                                return true;
                            }
                        }
                    }
                }
                UUID = E.parent;
            } else {
                break;
            }
        }
        Rules = Daemon()->FirmwareRules();
        return false;
    }

    bool InventoryDB::FindFirmwareOptionsForVenue(const std::string &VenueUUID, ProvObjects::FIRMWARE_UPGRADE_RULES & Rules) {
        std::string UUID = VenueUUID;
        while(!UUID.empty()) {
            ProvObjects::Venue                 V;
            if(StorageService()->VenueDB().GetRecord("id",UUID,V)) {
                if(!V.deviceConfiguration.empty()) {
                    ProvObjects::DeviceConfiguration    C;
                    for(const auto &i:V.deviceConfiguration) {
                        if(StorageService()->ConfigurationDB().GetRecord("id",i,C)) {
                            if(C.firmwareUpgrade=="no") {
                                Rules = ProvObjects::dont_upgrade;
                                return false;
                            }
                            if(C.firmwareUpgrade=="yes") {
                                if(C.firmwareRCOnly)
                                    Rules = ProvObjects::upgrade_release_only;
                                else
                                    Rules = ProvObjects::upgrade_latest;
                                return true;
                            }
                        }
                    }
                }
                if(!V.entity.empty()) {
                    return FindFirmwareOptionsForEntity(V.entity,Rules);
                } else {
                    UUID = V.parent;
                }
            } else {
                break;
            }
        }
        Rules = Daemon()->FirmwareRules();
        return false;
    }

    bool InventoryDB::FindFirmwareOptions(std::string &SerialNumber, ProvObjects::FIRMWARE_UPGRADE_RULES & Rules) {
        ProvObjects::InventoryTag   T;
        if(GetRecord("serialNumber",SerialNumber,T)) {
            std::cout << "SerialNumber: " << SerialNumber << " found device." << std::endl;
            //  if there is a local configuration, use this
            if(!T.deviceConfiguration.empty()) {
                ProvObjects::DeviceConfiguration    C;
                if(StorageService()->ConfigurationDB().GetRecord("id",T.deviceConfiguration,C)) {
                    if(C.firmwareUpgrade=="no") {
                        Rules = ProvObjects::dont_upgrade;
                        return false;
                    }
                    if(C.firmwareUpgrade=="yes") {
                        if(C.firmwareRCOnly)
                            Rules = ProvObjects::upgrade_release_only;
                        else
                            Rules = ProvObjects::upgrade_latest;
                        return true;
                    }
                }
            }

            //  look at entity...
            if(!T.entity.empty()) {
                return FindFirmwareOptionsForEntity(T.entity,Rules);
            }

            if(!T.venue.empty()) {
                return FindFirmwareOptionsForVenue(T.venue,Rules);
            }
            Rules = Daemon()->FirmwareRules();
            return false;
        }
        Rules = ProvObjects::dont_upgrade;
        return false;
    }

    void InventoryDB::InitializeSerialCache() {
        auto F = []( const ProvObjects::InventoryTag & T) ->bool { SerialNumberCache()->AddSerialNumber(T.serialNumber, T.deviceType); return true;};
        Iterate(F);
    }

    bool InventoryDB::LookForRRMInEntity(const std::string &Entity) {
        try {
            ProvObjects::Entity  E;
            if(StorageService()->EntityDB().GetRecord("id", Entity, E)) {
                if(E.rrm == "inherit") {
                    if(!E.parent.empty())
                        return LookForRRMInEntity(E.parent);
                    return false;
                }
                if(E.rrm=="no")
                    return false;
                return true;
            }
            return false;
        } catch(...) {

        }
        return false;
    }

    bool InventoryDB::LookForRRMInVenue(const std::string &Venue) {
        try {
            ProvObjects::Venue  V;
            if(StorageService()->VenueDB().GetRecord("id", Venue, V)) {
                if(V.rrm == "inherit") {
                    if(!V.parent.empty())
                        return LookForRRMInVenue(V.parent);
                    if(!V.entity.empty())
                        return LookForRRMInEntity(V.entity);
                    return false;
                }
                if(V.rrm=="no")
                    return false;
                return true;
            }
            return false;
        } catch(...) {

        }
        return false;
    }

    bool InventoryDB::LookForRRM(const ProvObjects::InventoryTag &T) {
        if(!T.venue.empty())
            return LookForRRMInVenue(T.venue);
        if(!T.entity.empty())
            return LookForRRMInEntity(T.entity);
        return false;
    }

    bool InventoryDB::GetRRMDeviceList(Types::UUIDvec_t &DeviceList) {
        // get a local copy of the cache - this could be expensive.
        auto C = SerialNumberCache()->GetCacheCopy();

        for(const auto &i:C) {
            std::string     SerialNumber = Utils::IntToSerialNumber(i.SerialNumber);
            ProvObjects::InventoryTag   Tag;
            if(StorageService()->InventoryDB().GetRecord("serialNumber",SerialNumber,Tag)) {
                if(Tag.rrm=="no")
                    continue;
                if(Tag.rrm=="inherit") {
                    if(!LookForRRM(Tag))
                        continue;
                }
                DeviceList.push_back(SerialNumber);
            }
        }
        return true;
    }
}

template<> void ORM::DB<    OpenWifi::InventoryDBRecordType, OpenWifi::ProvObjects::InventoryTag>::Convert(const OpenWifi::InventoryDBRecordType &In, OpenWifi::ProvObjects::InventoryTag &Out) {
    Out.info.id = In.get<0>();
    Out.info.name = In.get<1>();
    Out.info.description = In.get<2>();
    Out.info.notes = OpenWifi::RESTAPI_utils::to_object_array<OpenWifi::SecurityObjects::NoteInfo>(In.get<3>());
    Out.info.created = In.get<4>();
    Out.info.modified = In.get<5>();
    Out.serialNumber = In.get<6>();
    Out.venue = In.get<7>();
    Out.entity = In.get<8>();
    Out.subscriber = In.get<9>();
    Out.deviceType = In.get<10>();
    Out.qrCode = In.get<11>();
    Out.geoCode = In.get<12>();
    Out.location = In.get<13>();
    Out.contact = In.get<14>();
    Out.deviceConfiguration = In.get<15>();
    Out.rrm = In.get<16>();
    Out.info.tags = OpenWifi::RESTAPI_utils::to_taglist(In.get<17>());
    Out.managementPolicy = In.get<18>();
    Out.state = In.get<19>();
    Out.devClass = In.get<20>();
    Out.locale = In.get<21>();
    Out.realMacAddress = In.get<22>();
}

template<> void ORM::DB<    OpenWifi::InventoryDBRecordType, OpenWifi::ProvObjects::InventoryTag>::Convert(const OpenWifi::ProvObjects::InventoryTag &In, OpenWifi::InventoryDBRecordType &Out) {
    Out.set<0>(In.info.id);
    Out.set<1>(In.info.name);
    Out.set<2>(In.info.description);
    Out.set<3>(OpenWifi::RESTAPI_utils::to_string(In.info.notes));
    Out.set<4>(In.info.created);
    Out.set<5>(In.info.modified);
    Out.set<6>(In.serialNumber);
    Out.set<7>(In.venue);
    Out.set<8>(In.entity);
    Out.set<9>(In.subscriber);
    Out.set<10>(In.deviceType);
    Out.set<11>(In.qrCode);
    Out.set<12>(In.geoCode);
    Out.set<13>(In.location);
    Out.set<14>(In.contact);
    Out.set<15>(In.deviceConfiguration);
    Out.set<16>(In.rrm);
    Out.set<17>(OpenWifi::RESTAPI_utils::to_string(In.info.tags));
    Out.set<18>(In.managementPolicy);
    Out.set<19>(In.state);
    Out.set<20>(In.devClass);
    Out.set<21>(In.locale);
    Out.set<22>(In.realMacAddress);
}
