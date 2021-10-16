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
#include "RESTAPI_errors.h"

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
            for(const auto &i:DevUUIDS) {
                ProvObjects::InventoryTag E;
                if(Storage()->InventoryDB().GetRecord("id",i,E)) {
                    Tags.push_back(E);
                } else {
                    return BadRequest(RESTAPI::Errors::UnknownId + " (" + i + ")");
                }
            }
            return ReturnObject( "taglist", Tags);
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

                if(QB_.AdditionalInfo) {
                    Poco::JSON::Object  EI;
                    if(!i.entity.empty()) {
                        Poco::JSON::Object  EntObj;
                        ProvObjects::Entity Entity;
                        if(Storage()->EntityDB().GetRecord("id",i.entity,Entity)) {
                            EntObj.set( "name", Entity.info.name);
                            EntObj.set( "description", Entity.info.description);
                        }
                        EI.set("entity",EntObj);
                    }
                    if(!i.managementPolicy.empty()) {
                        Poco::JSON::Object  PolObj;
                        ProvObjects::ManagementPolicy Policy;
                        if(Storage()->PolicyDB().GetRecord("id",i.managementPolicy,Policy)) {
                            PolObj.set( "name", Policy.info.name);
                            PolObj.set( "description", Policy.info.description);
                        }
                        EI.set("managementPolicy",PolObj);
                    }
                    if(!i.venue.empty()) {
                        Poco::JSON::Object  EntObj;
                        ProvObjects::Venue Venue;
                        if(Storage()->VenueDB().GetRecord("id",i.venue,Venue)) {
                            EntObj.set( "name", Venue.info.name);
                            EntObj.set( "description", Venue.info.description);
                        }
                        EI.set("venue",EntObj);
                    }
                    if(!i.contact.empty()) {
                        Poco::JSON::Object  EntObj;
                        ProvObjects::Contact Contact;
                        if(Storage()->ContactDB().GetRecord("id",i.contact,Contact)) {
                            EntObj.set( "name", Contact.info.name);
                            EntObj.set( "description", Contact.info.description);
                        }
                        EI.set("contact",EntObj);
                    }
                    if(!i.location.empty()) {
                        Poco::JSON::Object  EntObj;
                        ProvObjects::Location Location;
                        if(Storage()->LocationDB().GetRecord("id",i.location,Location)) {
                            EntObj.set( "name", Location.info.name);
                            EntObj.set( "description", Location.info.description);
                        }
                        EI.set("location",EntObj);
                    }
                    if(!i.deviceConfiguration.empty()) {
                        Poco::JSON::Object  EntObj;
                        ProvObjects::DeviceConfiguration DevConf;
                        if(Storage()->ConfigurationDB().GetRecord("id",i.deviceConfiguration,DevConf)) {
                            EntObj.set( "name", DevConf.info.name);
                            EntObj.set( "description", DevConf.info.description);
                        }
                        EI.set("deviceConfiguration",EntObj);
                    }
                    O.set("extendedInfo", EI);
                }
                Arr.add(O);
            }
            Poco::JSON::Object  Answer;
            Answer.set("taglist",Arr);
            return ReturnObject(Answer);
        }
    }
}