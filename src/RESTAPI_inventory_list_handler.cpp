//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//


#include "RESTAPI_inventory_list_handler.h"
#include "StorageService.h"
#include "Utils.h"

namespace OpenWifi{
    void RESTAPI_inventory_list_handler::SendList( const ProvObjects::InventoryTagVec & Tags, bool SerialOnly) {
        Poco::JSON::Array   Array;
        for(const auto &i:Tags) {
            if(SerialOnly) {
                Array.add(i.serialNumber);
            } else {
                Poco::JSON::Object  O;
                i.to_json(O);
                Array.add(O);
            }
        }
        Poco::JSON::Object  Answer;
        if(SerialOnly)
            Answer.set("serialNumbers", Array);
        else
            Answer.set("tags", Array);
        ReturnObject(Answer);
    }

    void RESTAPI_inventory_list_handler::DoGet() {

        try {
            std::string UUID;
            std::string Arg;
            bool AddAdditionalInfo=false;
            if(HasParameter("withExtendedInfo",Arg) && Arg=="true")
                AddAdditionalInfo = true;

            bool SerialOnly=false;
            if(HasParameter("serialOnly",Arg) && Arg=="true")
                SerialOnly=true;

            if(!QB_.Select.empty()) {
                auto DevUUIDS = Utils::Split(QB_.Select);
                ProvObjects::InventoryTagVec Tags;
                for(const auto &i:DevUUIDS) {
                    ProvObjects::InventoryTag E;
                    if(Storage()->InventoryDB().GetRecord("id",i,E)) {
                        Tags.push_back(E);
                    } else {
                        BadRequest("Unknown UUID:" + i);
                        return;
                    }
                }
                ReturnObject( "tags", Tags);
                return;
            } else if(HasParameter("entity",UUID)) {
                if(QB_.CountOnly) {
                    auto C = Storage()->InventoryDB().Count( Storage()->InventoryDB().OP("entity",ORM::EQ,UUID));
                    ReturnCountOnly( C);
                    return;
                }
                ProvObjects::InventoryTagVec Tags;
                Storage()->InventoryDB().GetRecords(QB_.Offset, QB_.Limit, Tags, Storage()->InventoryDB().OP("entity",ORM::EQ,UUID));
                SendList(Tags, SerialOnly);
                return;
            } else if(HasParameter("venue",UUID)) {
                if(QB_.CountOnly) {
                    auto C = Storage()->InventoryDB().Count(Storage()->InventoryDB().OP("venue",ORM::EQ,UUID));
                    ReturnCountOnly( C);
                    return;
                }
                ProvObjects::InventoryTagVec Tags;
                Storage()->InventoryDB().GetRecords(QB_.Offset, QB_.Limit, Tags, Storage()->InventoryDB().OP("venue",ORM::EQ,UUID));
                SendList( Tags, SerialOnly);
                return;
            } else if(HasParameter("unassigned",Arg) && Arg=="true") {
                if(QB_.CountOnly) {
                    std::string Empty;
                    auto C = Storage()->InventoryDB().Count( InventoryDB::OP( Storage()->InventoryDB().OP("venue",ORM::EQ,Empty),
                                                                              ORM::AND, Storage()->InventoryDB().OP("entity",ORM::EQ,Empty) ));
                    ReturnCountOnly(C);
                    return;
                }
                ProvObjects::InventoryTagVec Tags;
                std::string Empty;
                Storage()->InventoryDB().GetRecords(QB_.Offset, QB_.Limit, Tags, InventoryDB::OP( Storage()->InventoryDB().OP("venue",ORM::EQ,Empty),
                                                                                                  ORM::AND, Storage()->InventoryDB().OP("entity",ORM::EQ,Empty) ) );
                SendList(Tags, SerialOnly);
                return;
            } else if(QB_.CountOnly) {
                auto C = Storage()->InventoryDB().Count();
                ReturnCountOnly(C);
                return;
            } else {
                ProvObjects::InventoryTagVec Tags;
                Storage()->InventoryDB().GetRecords(QB_.Offset,QB_.Limit,Tags);
                Poco::JSON::Array   Arr;
                for(const auto &i:Tags) {
                    Poco::JSON::Object  O;
                    i.to_json(O);
                    if(AddAdditionalInfo) {
                        if(!i.entity.empty()) {
                            Poco::JSON::Object  EO;
                            ProvObjects::Entity EI;
                            Storage()->EntityDB().GetRecord("id",i.entity,EI);
                            EI.to_json(EO);
                            O.set("entity_info",EO);
                        }
                        if(!i.venue.empty()) {
                            Poco::JSON::Object VO;
                            ProvObjects::Venue VI;
                            Storage()->VenueDB().GetRecord("id",i.venue,VI);
                            VI.to_json(VO);
                            O.set("venue_info",VO);
                        }
                    }
                    Arr.add(O);
                }
                Poco::JSON::Object  Answer;
                Answer.set("tags",Arr);
                ReturnObject(Answer);
                return;
            }
        } catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest("Internal error.");
    }

    void RESTAPI_inventory_list_handler::DoDelete() {}
    void RESTAPI_inventory_list_handler::DoPut() {}
    void RESTAPI_inventory_list_handler::DoPost() {}

}