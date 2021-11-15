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
#include "SDK_stubs.h"
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
        ORM::Field{"managementPolicy",ORM::FieldType::FT_TEXT}
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

    InventoryDB::InventoryDB( OpenWifi::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L) :
        DB(T, "inventory", InventoryDB_Fields, InventoryDB_Indexes, P, L, "inv") {}

        bool InventoryDB::CreateFromConnection(const std::string &SerialNumber, const std::string &ConnectionInfo,
                                          const std::string &DeviceType) {
        std::string SNum{SerialNumber};
        if(!Exists("serialNumber",SNum)) {
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
                    Poco::JSON::Object::Ptr Response;
                    if(SDK::DeviceSetVenue(NewDevice.serialNumber,FullUUID,Response)) {
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

}

template<> void ORM::DB<    OpenWifi::InventoryDBRecordType, OpenWifi::ProvObjects::InventoryTag>::Convert(OpenWifi::InventoryDBRecordType &In, OpenWifi::ProvObjects::InventoryTag &Out) {
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
}

template<> void ORM::DB<    OpenWifi::InventoryDBRecordType, OpenWifi::ProvObjects::InventoryTag>::Convert(OpenWifi::ProvObjects::InventoryTag &In, OpenWifi::InventoryDBRecordType &Out) {
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
}
