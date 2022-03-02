//
// Created by stephane bourque on 2021-10-18.
//

#pragma once

#include "RESTObjects/RESTAPI_ProvObjects.h"
#include "StorageService.h"
#include "framework/MicroService.h"

namespace OpenWifi {

    static void AddInfoBlock(const ProvObjects::ObjectInfo & O, Poco::JSON::Object &J) {
        J.set("name", O.name);
        J.set("description", O.description);
        J.set("id", O.id);
    }

    template <typename R, typename Q = decltype(R{}.entity)> void Extend_entity(const R &T, Poco::JSON::Object &EI ) {
        if constexpr(std::is_same_v<Q,std::string>) {
            if(!T.entity.empty()) {
                Poco::JSON::Object  EntObj;
                ProvObjects::Entity Entity;
                if(StorageService()->EntityDB().GetRecord("id",T.entity,Entity)) {
                    AddInfoBlock(Entity.info, EntObj);
                }
                EI.set("entity",EntObj);
            }
        }
    }
    template <typename... Ts> void Extend_entity(Ts... args) {
        static_assert( sizeof...(args) == 2);
    }

    template <typename R, typename Q = decltype(R{}.managementPolicy)> void Extend_managementPolicy(const R &T, Poco::JSON::Object &EI ) {
        if constexpr(std::is_same_v<Q,std::string>) {
            if(!T.managementPolicy.empty()) {
                Poco::JSON::Object  PolObj;
                ProvObjects::ManagementPolicy Policy;
                if(StorageService()->PolicyDB().GetRecord("id",T.managementPolicy,Policy)) {
                    AddInfoBlock(Policy.info, PolObj);
                }
                EI.set("managementPolicy",PolObj);
            }
        }
    }
    template <typename... Ts> void Extend_managementPolicy(Ts... args) {
        static_assert( sizeof...(args) == 2);
    }

    template <typename R, typename Q = decltype(R{}.venue)> void Extend_venue(const R &T, Poco::JSON::Object &EI ) {
        if constexpr(std::is_same_v<Q,std::string>) {
            if(!T.venue.empty()) {
                Poco::JSON::Object  VenObj;
                ProvObjects::Venue Venue;
                if(StorageService()->VenueDB().GetRecord("id",T.venue,Venue)) {
                    AddInfoBlock(Venue.info, VenObj);
                }
                EI.set("venue",VenObj);
            }
        }
    }
    template <typename... Ts> void Extend_venue(Ts... args) {
        static_assert( sizeof...(args) == 2);
    }

    template <typename R, typename Q = decltype(R{}.contact)> void Extend_contact(const R &T, Poco::JSON::Object &EI ) {
        if constexpr(std::is_same_v<Q,std::string>) {
            if(!T.contact.empty()) {
                Poco::JSON::Object  ConObj;
                ProvObjects::Contact Contact;
                if(StorageService()->ContactDB().GetRecord("id",T.contact,Contact)) {
                    AddInfoBlock(Contact.info, ConObj);
                }
                EI.set("contact",ConObj);
            }
        }
    }
    template <typename... Ts> void Extend_contact(Ts... args) {
        static_assert( sizeof...(args) == 2);
    }

    template <typename R, typename Q = decltype(R{}.location)> void Extend_location(const R &T, Poco::JSON::Object &EI ) {
        if constexpr(std::is_same_v<Q,std::string>) {
            if(!T.location.empty()) {
                Poco::JSON::Object  LocObj;
                ProvObjects::Location Location;
                if(StorageService()->LocationDB().GetRecord("id",T.location,Location)) {
                    AddInfoBlock(Location.info, LocObj);
                }
                EI.set("location",LocObj);
            }
        }
    }
    template <typename... Ts> void Extend_location(Ts... args) {
        static_assert( sizeof...(args) == 2);
    }

    template <typename R, typename Q = decltype(R{}.deviceConfiguration)> void Extend_deviceConfiguration(const R &T, Poco::JSON::Object &EI ) {
        if constexpr(std::is_same_v<Q,std::string>) {
            if(!T.deviceConfiguration.empty()) {
                Poco::JSON::Object  DevObj;
                ProvObjects::DeviceConfiguration DevConf;
                if(StorageService()->ConfigurationDB().GetRecord("id",T.deviceConfiguration,DevConf)) {
                    AddInfoBlock(DevConf.info, DevObj);
                }
                EI.set("deviceConfiguration",DevObj);
            }
        }
        if constexpr(std::is_same_v<Q,Types::UUIDvec_t>) {
            if(!T.deviceConfiguration.empty()) {
                Poco::JSON::Array  ObjArr;
                ProvObjects::DeviceConfiguration DevConf;
                for(const auto &i:T.deviceConfiguration) {
                    if(StorageService()->ConfigurationDB().GetRecord("id",i,DevConf)) {
                        Poco::JSON::Object  InnerObj;
                        AddInfoBlock(DevConf.info, InnerObj);
                        ObjArr.add(InnerObj);
                    }
                }
                EI.set("deviceConfiguration",ObjArr);
            }
        }
    }

    template <typename... Ts> void Extend_deviceConfiguration(Ts... args) {
        static_assert( sizeof...(args) == 2);
    }

    template <typename R> bool AddExtendedInfo(const R & T, Poco::JSON::Object &O) {
        Poco::JSON::Object  EI;
        Extend_entity(T,EI);
        Extend_deviceConfiguration(T,EI);
        Extend_location(T,EI);
        Extend_contact(T,EI);
        Extend_venue(T,EI);
        Extend_managementPolicy(T,EI);
        O.set("extendedInfo", EI);
        return true;
    }

    template <typename T> void MakeJSONObjectArray(const char * ArrayName, const std::vector<T> & V, RESTAPIHandler & R) {
        Poco::JSON::Array   ObjArray;
        for(const auto &i:V) {
            Poco::JSON::Object  Obj;
            i.to_json(Obj);
            if(R.NeedAdditionalInfo())
                AddExtendedInfo(i,Obj);
            ObjArray.add(Obj);
        }
        Poco::JSON::Object  Answer;
        Answer.set(ArrayName,ObjArray);
        return R.ReturnObject(Answer);
    }

//    ReturnRecordList<decltype(StorageService()->InventoryDB()),
//            ProvObjects::InventoryTag>("taglist",StorageService()->InventoryDB(),*this );

    inline static bool is_uuid(const std::string &u) {
        return u.find('-') != std::string::npos;
    }

    template <typename DB> void ReturnRecordList(const char *ArrayName,DB & DBInstance, RESTAPIHandler & R) {
        Poco::JSON::Array   ObjArr;
        for(const auto &i:R.SelectedRecords()) {
            ProvObjects::InventoryTag E;
            if(DBInstance.GetRecord(is_uuid(i) ? "id" : "serialNumber",i,E)) {
                Poco::JSON::Object  Obj;
                E.to_json(Obj);
                if(R.NeedAdditionalInfo())
                    AddExtendedInfo(E,Obj);
                ObjArr.add(Obj);
            } else {
                return R.BadRequest(RESTAPI::Errors::UnknownId + i);
            }
        }
        Poco::JSON::Object  Answer;
        Answer.set(ArrayName, ObjArr);
        return R.ReturnObject(Answer);
    }

    template <typename DB, typename Record> void ReturnRecordList(const char *ArrayName,DB & DBInstance, RESTAPIHandler & R) {
        Poco::JSON::Array   ObjArr;
        for(const auto &i:R.SelectedRecords()) {
            Record E;
            if(DBInstance.GetRecord("id",i,E)) {
                Poco::JSON::Object  Obj;
                E.to_json(Obj);
                if(R.NeedAdditionalInfo())
                    AddExtendedInfo(E,Obj);
                ObjArr.add(Obj);
            } else {
                return R.BadRequest(RESTAPI::Errors::UnknownId + i);
            }
        }
        Poco::JSON::Object  Answer;
        Answer.set(ArrayName, ObjArr);
        return R.ReturnObject(Answer);
    }

    inline bool NormalizeMac(std::string & Mac) {
        Poco::replaceInPlace(Mac,":","");
        Poco::replaceInPlace(Mac,"-","");
        if(Mac.size()!=12)
            return false;
        for(const auto &i:Mac) {
            if(!std::isxdigit(i))
                return false;
        }
        Poco::toLowerInPlace(Mac);
        return true;
    }

    template <typename DB> void ListHandler(const char *BlockName,DB & DBInstance, RESTAPIHandler & R) {
        auto Entity = R.GetParameter("entity", "");
        auto Venue = R.GetParameter("venue", "");

        typedef typename DB::RecordVec      RecVec;
        typedef typename DB::RecordName     RecType;

        if(!R.QB_.Select.empty()) {
            return ReturnRecordList<decltype(DBInstance),
                    RecType>(BlockName, DBInstance, R);
        } if(!Entity.empty()) {
            RecVec Entries;
            DBInstance.GetRecords(R.QB_.Offset,R.QB_.Limit,Entries," entity=' " + Entity +"'");
            if(R.QB_.CountOnly)
                return R.ReturnCountOnly(Entries.size());
            return MakeJSONObjectArray(BlockName, Entries, R);
        } if(!Venue.empty()) {
            RecVec Entries;
            DBInstance.GetRecords(R.QB_.Offset,R.QB_.Limit,Entries," venue=' " + Venue +"'");
            if(R.QB_.CountOnly)
                return R.ReturnCountOnly(Entries.size());
            return MakeJSONObjectArray(BlockName, Entries, R);
        } else if(R.QB_.CountOnly) {
            Poco::JSON::Object  Answer;
            auto C = DBInstance.Count();
            return R.ReturnCountOnly(C);
        } else {
            RecVec Entries;
            DBInstance.GetRecords(R.QB_.Offset,R.QB_.Limit,Entries);
            return MakeJSONObjectArray(BlockName, Entries, R);
        }
    }

    template <typename DBUsage, typename ObjectDB> void MoveUsage(DBUsage &DB_InUse, ObjectDB & DB, const std::string & From, const std::string & To, const std::string &Id) {
        if(From!=To) {
            if(!From.empty())
                DB_InUse.DeleteInUse("id",From,DB.Prefix(),Id);
            if(!To.empty())
                DB_InUse.AddInUse("id",To,DB.Prefix(),Id);
        }
    }

    template <typename DBUsage, typename ObjectDB> void MoveUsage(DBUsage &DB_InUse, ObjectDB & DB, const Types::UUIDvec_t & From, const Types::UUIDvec_t & To, const std::string &Id) {
        if(From!=To) {
            if(!From.empty()) {
                for(const auto &i:From)
                    DB_InUse.DeleteInUse("id", i, DB.Prefix(), Id);
            }
            if(!To.empty()) {
                for(const auto &i:To)
                    DB_InUse.AddInUse("id", i, DB.Prefix(), Id);
            }
        }
    }

    template <typename DB> void MoveChild(DB &TheDB, const std::string & Parent, const std::string & Child, const std::string &Id) {
        if(Parent!=Child) {
            if(!Parent.empty())
                TheDB.InUse.DeleteInUse("id",Parent,Id);
            if(!Child.empty())
                TheDB.AddInUse("id",Child,Id);
        }
    }

    template <typename DB, typename Member> void RemoveMembership( DB & TheDB, Member T, const std::string & Obj, const std::string &Id) {
        if(!Obj.empty())
            TheDB.ManipulateVectorMember(T, "id", Obj, Id, false);
    }

    template <typename DB, typename Member> void AddMembership( DB & TheDB, Member T, const std::string & Obj, const std::string &Id) {
        if(!Obj.empty())
            TheDB.ManipulateVectorMember(T, "id", Obj, Id, true);
    }

    template <typename DB, typename Member> void ManageMembership( DB & TheDB, Member T, const std::string & From, const std::string & To, const std::string &Id) {
        RemoveMembership(TheDB,T,From,Id);
        AddMembership(TheDB,T,To,Id);
    }

    template <typename DB, typename Member> void ManageMembership( DB & TheDB, Member T, const Types::UUIDvec_t & From, const Types::UUIDvec_t & To, const std::string &Id) {
        if(From!=To) {
            for (const auto &i: From)
                RemoveMembership(TheDB, T, i, Id);
            for (const auto &i: To)
                AddMembership(TheDB, T, i, Id);
        }
    }

    template <typename Member, typename Rec, typename DB > bool CreateMove(const Poco::JSON::Object::Ptr & RawObj, const char *fieldname, Member T, Rec & Existing, std::string &From, std::string &To, DB & TheDB) {
        if(RawObj->has(fieldname)) {
            From = Existing.*T;
            To = RawObj->get(fieldname).toString();
            if(!To.empty() && !TheDB.Exists("id",To))
                return false;
            Existing.*T=To;
        }
        return true;
    }
}
