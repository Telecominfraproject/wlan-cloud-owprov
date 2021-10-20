//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//


#include "RESTAPI_entity_list_handler.h"
#include "framework/Utils.h"
#include "StorageService.h"
#include "framework/RESTAPI_utils.h"
#include "framework/RESTAPI_errors.h"
#include "RESTAPI_db_helpers.h"

namespace OpenWifi{

    void RESTAPI_entity_list_handler::DoGet() {
        std::string Arg;
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
            Poco::JSON::Array   ObjArray;
            for(const auto &i:Entities) {
                Poco::JSON::Object  O;
                i.to_json(O);
                if(QB_.AdditionalInfo)
                    AddEntityExtendedInfo( i, O);
                ObjArray.add(O);
            }
            Poco::JSON::Object  Answer;
            Answer.set("entities",ObjArray);
            return ReturnObject(Answer);
        }
    }

    void RESTAPI_entity_list_handler::DoPost() {
        std::string Arg;
        if (HasParameter("setTree",Arg) && Arg=="true") {
            auto FullTree = ParseStream();
            Storage()->EntityDB().ImportTree(FullTree);
            return OK();
        }
        BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
    }
}