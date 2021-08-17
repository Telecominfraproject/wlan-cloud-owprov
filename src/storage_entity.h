//
// Created by stephane bourque on 2021-08-15.
//

#ifndef OWPROV_STORAGE_ENTITY_H
#define OWPROV_STORAGE_ENTITY_H

#include "orm.h"
#include "RESTAPI_ProvObjects.h"

namespace OpenWifi {

    /*
        ORM::Field{"id",64, true},
        ORM::Field{"name",ORM::FieldType::FT_TEXT},
        ORM::Field{"description",ORM::FieldType::FT_TEXT},
        ORM::Field{"notes",ORM::FieldType::FT_TEXT},
        ORM::Field{"created",ORM::FieldType::FT_BIGINT},
        ORM::Field{"modified",ORM::FieldType::FT_BIGINT},
        ORM::Field{"parent",ORM::FieldType::FT_TEXT},
        ORM::Field{"children",ORM::FieldType::FT_TEXT},
        ORM::Field{"managers",ORM::FieldType::FT_TEXT},
        ORM::Field{"contacts",ORM::FieldType::FT_TEXT},
        ORM::Field{"locations",ORM::FieldType::FT_TEXT},
        ORM::Field{"managementPolicy",ORM::FieldType::FT_TEXT}
     */
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
                std::string
            > EntityDBRecordType;

    class EntityDB : public ORM::DB<EntityDBRecordType, ProvObjects::Entity> {
    public:
        EntityDB( ORM::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L);
    private:
    };
}


#endif //OWPROV_STORAGE_ENTITY_H
