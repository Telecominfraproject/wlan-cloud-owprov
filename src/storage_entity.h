//
// Created by stephane bourque on 2021-08-15.
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
    private:
        bool RootExists_=false;
    };
}


#endif //OWPROV_STORAGE_ENTITY_H
