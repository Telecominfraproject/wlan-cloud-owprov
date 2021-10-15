//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//


#include "RESTAPI_entity_list_handler.h"
#include "Utils.h"
#include "StorageService.h"
#include "RESTAPI_utils.h"
#include "RESTAPI_errors.h"

namespace OpenWifi{

    void RESTAPI_entity_list_handler::DoGet() {
        std::string Arg;
        bool AdditionalInfo = HasParameter("withExtendedInfo",Arg) && Arg=="true";
        if(!QB_.Select.empty()) {
            auto EntityUIDs = Utils::Split(QB_.Select);
            ProvObjects::EntityVec Entities;
            for(const auto &i:EntityUIDs) {
                ProvObjects::Entity E;
                if(Storage()->EntityDB().GetRecord("id",i,E)) {
                    Entities.push_back(E);
                } else {
                    return BadRequest(RESTAPI::Errors::UnknownId + " (" + i + ")");
                }
            }
            return ReturnObject("entities", Entities);
        } else if(QB_.CountOnly) {
            auto C = Storage()->EntityDB().Count();
            return ReturnCountOnly(C);
        } if (HasParameter("getTree",Arg) && Arg=="true") {
            Poco::JSON::Object  FullTree;
            Storage()->EntityDB().BuildTree(FullTree);
            return ReturnObject(FullTree);
        } else {
            ProvObjects::EntityVec Entities;
            Storage()->EntityDB().GetRecords(QB_.Offset, QB_.Limit,Entities);
            if(AdditionalInfo) {
                Poco::JSON::Array   ObjArray;
                for(const auto &i:Entities) {
                    Poco::JSON::Object  O;
                    i.to_json(O);
                    Poco::JSON::Object  EI;
                    if(!i.managementPolicy.empty()) {
                        Poco::JSON::Object  PolObj;
                        ProvObjects::ManagementPolicy Policy;
                        if(Storage()->PolicyDB().GetRecord("id",i.managementPolicy,Policy)) {
                            PolObj.set( "name", Policy.info.name);
                            PolObj.set( "description", Policy.info.description);
                        }
                        EI.set("managementPolicy",PolObj);
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
                    ObjArray.add(O);
                }
                Poco::JSON::Object  Answer;
                Answer.set("entities",ObjArray);
                return ReturnObject(Answer);
            }
            return ReturnObject("entities",Entities);
        }
    }

    void RESTAPI_entity_list_handler::DoPost() {
        std::string Arg;
        if (HasParameter("setTree",Arg) && Arg=="true") {
            auto FullTree = ParseStream();
            Storage()->EntityDB().ImportTree(FullTree);
            OK();
            return;
        }
        BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
    }
}