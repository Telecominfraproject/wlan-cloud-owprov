//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//


#ifndef OWPROV_STORAGE_ENTITY_H
#define OWPROV_STORAGE_ENTITY_H

#include "orm.h"
#include "RESTAPI_ProvObjects.h"

namespace OpenWifi {

    typedef Poco::Tuple<
                std::string,
                std::string,
                std::string,
                std::string,
                uint64_t,
                uint64_t,
                std::string,
                std::string,
                std::string,
                std::string,
                std::string,
                std::string,
                std::string,
                std::string
            > EntityDBRecordType;

    class EntityDB : public ORM::DB<EntityDBRecordType, ProvObjects::Entity> {
    public:
        static const std::string RootUUID_;
        EntityDB( ORM::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L);
        inline bool RootExists() const { return RootExists_; };
        static inline bool IsRoot(const std::string &UUID) { return (UUID == RootUUID_); }
        static inline const std::string RootUUID() { return RootUUID_; }
        bool CheckForRoot();
        void BuildTree(Poco::JSON::Object &Tree, const std::string & Node = RootUUID_ );
        void AddVenues(Poco::JSON::Object &Tree, const std::string & Venue );
        void ImportTree(const Poco::JSON::Object::Ptr &Ptr, const std::string & Node = RootUUID_ );
        void ImportVenues(const Poco::JSON::Object::Ptr &Ptr, const std::string & Node = RootUUID_ );
        bool CreateShortCut( ProvObjects::Entity & E);
    private:
        bool RootExists_=false;
    };
}


#endif //OWPROV_STORAGE_ENTITY_H
