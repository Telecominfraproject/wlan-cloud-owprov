//
// Created by stephane bourque on 2021-08-23.
//

#include "RESTAPI_venue_list_handler.h"
#include "Utils.h"
#include "StorageService.h"
#include "RESTAPI_utils.h"
#include "RESTAPI_errors.h"

namespace OpenWifi{
    void RESTAPI_venue_list_handler::DoGet() {
        try {
            std::string Arg;
            bool AdditionalInfo = HasParameter("withExtendedInfo",Arg) && Arg=="true";
            if(!QB_.Select.empty()) {
                auto UUIDs = Utils::Split(QB_.Select);
                ProvObjects::VenueVec  Venues;
                for(const auto &i:UUIDs) {
                    ProvObjects::Venue E;
                    if(Storage()->VenueDB().GetRecord("id",i,E)) {
                        Venues.push_back(E);
                    } else {
                        BadRequest("Unknown UUID:" + i);
                        return;
                    }
                }
                ReturnObject("venues", Venues);
                return;
            } else if(HasParameter("entity",Arg)) {
                ProvObjects::VenueVec Venues;
                Storage()->VenueDB().GetRecords(QB_.Offset,QB_.Limit,Venues, Storage()->VenueDB().OP("entity",ORM::EQ,Arg));
                if(QB_.CountOnly) {
                    ReturnCountOnly(Venues.size());
                } else {
                    ReturnObject("venues", Venues);
                }
                return;
            } else if(HasParameter("venue",Arg)) {
                ProvObjects::VenueVec Venues;
                Storage()->VenueDB().GetRecords(QB_.Offset,QB_.Limit,Venues,Storage()->VenueDB().OP("venue",ORM::EQ,Arg));
                if(QB_.CountOnly) {
                    ReturnCountOnly(Venues.size());
                } else {
                    ReturnObject("venues", Venues);
                }
                return;
            } else if(QB_.CountOnly) {
                Poco::JSON::Object  Answer;
                auto C = Storage()->VenueDB().Count();
                ReturnCountOnly(C);
                return;
            } else {
                ProvObjects::VenueVec Venues;
                Storage()->VenueDB().GetRecords(QB_.Offset, QB_.Limit,Venues);

                if(AdditionalInfo) {
                    Poco::JSON::Array   ObjArr;
                    for(const auto &i:Venues) {
                        Poco::JSON::Object  Obj;
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
                        Obj.set("extendedInfo", EI);
                        i.to_json(Obj);
                        ObjArr.add(Obj);
                    }
                    Poco::JSON::Object  Answer;
                    Answer.set("venues", ObjArr);
                    return ReturnObject(Answer);
                }
                return ReturnObject("venues", Venues);
            }
        } catch(const Poco::Exception &E) {
            Logger_.log(E);
        }
        InternalError("Internal error.");
    }
}