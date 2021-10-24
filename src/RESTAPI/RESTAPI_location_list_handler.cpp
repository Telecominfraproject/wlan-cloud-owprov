//
// Created by stephane bourque on 2021-08-23.
//

#include "RESTAPI_location_list_handler.h"

#include "RESTObjects/RESTAPI_ProvObjects.h"
#include "StorageService.h"
#include "framework/RESTAPI_errors.h"
#include "RESTAPI/RESTAPI_db_helpers.h"

namespace OpenWifi{

    void RESTAPI_location_list_handler::DoGet() {
        if(!QB_.Select.empty()) {
            auto DevUUIDS = Utils::Split(QB_.Select);
            Poco::JSON::Array   ObjArr;
            for(const auto &i:DevUUIDS) {
                ProvObjects::Location E;
                if(StorageService()->LocationDB().GetRecord("id",i,E)) {
                    Poco::JSON::Object  Obj;
                    E.to_json(Obj);
                    if(QB_.AdditionalInfo)
                        AddLocationExtendedInfo(E, Obj);
                    ObjArr.add(Obj);
                } else {
                    return BadRequest(RESTAPI::Errors::LocationMustExist + " ("+i+")");
                }
            }
            Poco::JSON::Object  Answer;
            Answer.set("locations", ObjArr);
            return ReturnObject(Answer);
        } else if(QB_.CountOnly) {
            Poco::JSON::Object  Answer;
            auto C = StorageService()->LocationDB().Count();
            return ReturnCountOnly(C);
        } else {
            ProvObjects::LocationVec Locations;
            StorageService()->LocationDB().GetRecords(QB_.Offset,QB_.Limit,Locations);

            Poco::JSON::Array   ObjArray;
            for(const auto &i:Locations) {
                Poco::JSON::Object  Obj;
                i.to_json(Obj);
                if(QB_.AdditionalInfo)
                    AddLocationExtendedInfo(i, Obj);
                ObjArray.add(Obj);
            }
            Poco::JSON::Object  Answer;
            Answer.set("locations",ObjArray);
            return ReturnObject(Answer);
        }
    }
}