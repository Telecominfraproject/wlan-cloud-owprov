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

namespace OpenWifi{

    void RESTAPI_entity_list_handler::DoGet() {
        try {
            std::string Arg;
            if(!QB_.Select.empty()) {
                auto EntityUIDs = Utils::Split(QB_.Select);
                ProvObjects::EntityVec Entities;
                for(const auto &i:EntityUIDs) {
                    ProvObjects::Entity E;
                    if(Storage()->EntityDB().GetRecord("id",i,E)) {
                        Entities.push_back(E);
                    } else {
                        BadRequest("Unknown UUID:" + i);
                        return;
                    }
                }
                ReturnObject("entities", Entities);
                return;
            } else if(QB_.CountOnly) {
                auto C = Storage()->EntityDB().Count();
                ReturnCountOnly(C);
                return;
            } if (HasParameter("getTree",Arg) && Arg=="true") {
                Poco::JSON::Object  FullTree;
                Storage()->EntityDB().BuildTree(FullTree);
                ReturnObject(FullTree);
                return;
            } else {
                ProvObjects::EntityVec Entities;
                Storage()->EntityDB().GetRecords(QB_.Offset, QB_.Limit,Entities);
                ReturnObject("entities",Entities);
                return;
            }
        } catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest("Internal error");
    }

    void RESTAPI_entity_list_handler::DoPost() {
        try {
            std::string Arg;
            if (HasParameter("setTree",Arg) && Arg=="true") {
                auto FullTree = ParseStream();
                Storage()->EntityDB().ImportTree(FullTree);
                OK();
                return;
            }
        } catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        BadRequest("Internal error. Please try again.");
    }

    void RESTAPI_entity_list_handler::DoPut() {};
    void RESTAPI_entity_list_handler::DoDelete() {};
}