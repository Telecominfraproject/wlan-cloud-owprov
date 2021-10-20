//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//


#include "RESTAPI_inventory_list_handler.h"
#include "StorageService.h"
#include "framework/Utils.h"
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
                    AddInventoryExtendedInfo(i,O);
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
            if(!Storage()->InventoryDB().PrepareOrderBy(Arg,OrderBy)) {
                return BadRequest(RESTAPI::Errors::InvalidLOrderBy);
            }
        }

        if(!QB_.Select.empty()) {
            auto DevUUIDS = Utils::Split(QB_.Select);
            ProvObjects::InventoryTagVec Tags;
            Poco::JSON::Array   ObjArr;
            for(const auto &i:DevUUIDS) {
                ProvObjects::InventoryTag E;
                if(Storage()->InventoryDB().GetRecord("id",i,E)) {
                    Poco::JSON::Object  O;
                    E.to_json(O);
                    if(QB_.AdditionalInfo)
                        AddInventoryExtendedInfo(E,O);
                    ObjArr.add(O);
                } else {
                    return BadRequest(RESTAPI::Errors::UnknownId + " (" + i + ")");
                }
            }
            Poco::JSON::Object  Answer;
            Answer.set("taglist",ObjArr);
            return ReturnObject( Answer);
        } else if(HasParameter("entity",UUID)) {
            if(QB_.CountOnly) {
                auto C = Storage()->InventoryDB().Count( Storage()->InventoryDB().OP("entity",ORM::EQ,UUID));
                return ReturnCountOnly( C);
            }
            ProvObjects::InventoryTagVec Tags;
            Storage()->InventoryDB().GetRecords(QB_.Offset, QB_.Limit, Tags, Storage()->InventoryDB().OP("entity",ORM::EQ,UUID), OrderBy);
            return SendList(Tags, SerialOnly);
        } else if(HasParameter("venue",UUID)) {
            if(QB_.CountOnly) {
                auto C = Storage()->InventoryDB().Count(Storage()->InventoryDB().OP("venue",ORM::EQ,UUID));
                return ReturnCountOnly( C);
            }
            ProvObjects::InventoryTagVec Tags;
            Storage()->InventoryDB().GetRecords(QB_.Offset, QB_.Limit, Tags, Storage()->InventoryDB().OP("venue",ORM::EQ,UUID), OrderBy);
            return SendList( Tags, SerialOnly);
        } else if(HasParameter("unassigned",Arg) && Arg=="true") {
            if(QB_.CountOnly) {
                std::string Empty;
                auto C = Storage()->InventoryDB().Count( InventoryDB::OP( Storage()->InventoryDB().OP("venue",ORM::EQ,Empty),
                                                                          ORM::AND, Storage()->InventoryDB().OP("entity",ORM::EQ,Empty) ));
                return ReturnCountOnly(C);
            }
            ProvObjects::InventoryTagVec Tags;
            std::string Empty;
            Storage()->InventoryDB().GetRecords(QB_.Offset, QB_.Limit, Tags, InventoryDB::OP( Storage()->InventoryDB().OP("venue",ORM::EQ,Empty),
                                                                                              ORM::AND, Storage()->InventoryDB().OP("entity",ORM::EQ,Empty) ) , OrderBy );
            return SendList(Tags, SerialOnly);
        } else if(QB_.CountOnly) {
            auto C = Storage()->InventoryDB().Count();
            return ReturnCountOnly(C);
        } else {
            ProvObjects::InventoryTagVec Tags;
            Storage()->InventoryDB().GetRecords(QB_.Offset,QB_.Limit,Tags,"",OrderBy);
            Poco::JSON::Array   Arr;

            for(const auto &i:Tags) {
                Poco::JSON::Object  O;
                i.to_json(O);

                if(QB_.AdditionalInfo)
                    AddInventoryExtendedInfo(i,O);
                Arr.add(O);
            }
            Poco::JSON::Object  Answer;
            Answer.set("taglist",Arr);
            return ReturnObject(Answer);
        }
    }
}