//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//


#include "RESTAPI_inventory_list_handler.h"
#include "StorageService.h"
#include "framework/RESTAPI_errors.h"
#include "RESTAPI/RESTAPI_db_helpers.h"

namespace OpenWifi{
    void RESTAPI_inventory_list_handler::SendList( const ProvObjects::InventoryTagVec & Tags, bool SerialOnly) {
        Poco::JSON::Array   Array;
        for(const auto &i:Tags) {
            if(SerialOnly) {
                Array.add(i.serialNumber);
            } else {
                Poco::JSON::Object  O;
                i.to_json(O);
                if(QB_.AdditionalInfo)
                    AddExtendedInfo(i,O);
                Array.add(O);
            }
        }
        Poco::JSON::Object  Answer;
        if(SerialOnly)
            Answer.set("serialNumbers", Array);
        else
            Answer.set("taglist", Array);
        ReturnObject(Answer);
    }

    void RESTAPI_inventory_list_handler::DoGet() {
        std::string UUID;
        std::string Arg;

        bool SerialOnly=false;
        if(HasParameter("serialOnly",Arg) && Arg=="true")
            SerialOnly=true;

        std::string OrderBy{" ORDER BY serialNumber ASC "};
        if(HasParameter("orderBy",Arg)) {
            if(!StorageService()->InventoryDB().PrepareOrderBy(Arg,OrderBy)) {
                return BadRequest(RESTAPI::Errors::InvalidLOrderBy);
            }
        }

        if(!QB_.Select.empty()) {
            return ReturnRecordList<decltype(StorageService()->InventoryDB()),
            ProvObjects::InventoryTag>("taglist",StorageService()->InventoryDB(),*this );
        } else if(HasParameter("entity",UUID)) {
            if(QB_.CountOnly) {
                auto C = StorageService()->InventoryDB().Count( StorageService()->InventoryDB().OP("entity",ORM::EQ,UUID));
                return ReturnCountOnly( C);
            }
            ProvObjects::InventoryTagVec Tags;
            StorageService()->InventoryDB().GetRecords(QB_.Offset, QB_.Limit, Tags, StorageService()->InventoryDB().OP("entity",ORM::EQ,UUID), OrderBy);
            return SendList(Tags, SerialOnly);
        } else if(HasParameter("venue",UUID)) {
            if(QB_.CountOnly) {
                auto C = StorageService()->InventoryDB().Count(StorageService()->InventoryDB().OP("venue",ORM::EQ,UUID));
                return ReturnCountOnly( C);
            }
            ProvObjects::InventoryTagVec Tags;
            StorageService()->InventoryDB().GetRecords(QB_.Offset, QB_.Limit, Tags, StorageService()->InventoryDB().OP("venue",ORM::EQ,UUID), OrderBy);
            return SendList( Tags, SerialOnly);
        } else if(HasParameter("unassigned",Arg) && Arg=="true") {
            if(QB_.CountOnly) {
                std::string Empty;
                auto C = StorageService()->InventoryDB().Count( InventoryDB::OP( StorageService()->InventoryDB().OP("venue",ORM::EQ,Empty),
                                                                          ORM::AND, StorageService()->InventoryDB().OP("entity",ORM::EQ,Empty) ));
                return ReturnCountOnly(C);
            }
            ProvObjects::InventoryTagVec Tags;
            std::string Empty;
            StorageService()->InventoryDB().GetRecords(QB_.Offset, QB_.Limit, Tags, InventoryDB::OP( StorageService()->InventoryDB().OP("venue",ORM::EQ,Empty),
                                                                                              ORM::AND, StorageService()->InventoryDB().OP("entity",ORM::EQ,Empty) ) , OrderBy );
            return SendList(Tags, SerialOnly);
        } else if (HasParameter("subscriber",Arg) && !Arg.empty()) {
            // looking for device(s) for a specific subscriber...
            ProvObjects::InventoryTagVec Devices;
            StorageService()->InventoryDB().GetRecords(0,100,Devices," subscriber='" + Arg + "'");
            return SendList(Devices,SerialOnly);
        } else if (QB_.CountOnly) {
            auto C = StorageService()->InventoryDB().Count();
            return ReturnCountOnly(C);
        } else if (GetBoolParameter("rrmOnly",false)) {
            Types::UUIDvec_t   DeviceList;
            StorageService()->InventoryDB().GetRRMDeviceList(DeviceList);
            if(QB_.CountOnly)
                return ReturnCountOnly(DeviceList.size());
            else {
                return ReturnObject("serialNumbers",DeviceList);
            }
        } else {
            ProvObjects::InventoryTagVec Tags;
            StorageService()->InventoryDB().GetRecords(QB_.Offset,QB_.Limit,Tags,"",OrderBy);
            return MakeJSONObjectArray("taglist", Tags, *this);
        }
    }
}