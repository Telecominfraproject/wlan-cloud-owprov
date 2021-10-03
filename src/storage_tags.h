//
// Created by stephane bourque on 2021-10-02.
//

#ifndef OWPROV_STORAGE_TAGS_H
#define OWPROV_STORAGE_TAGS_H

#include "orm.h"
#include "OpenWifiTypes.h"

namespace OpenWifi {
    struct TagsDictionary {
        std::string     entity;
        uint32_t        id=0;
        std::string     name;
    };

    typedef Poco::Tuple<
        std::string,
        uint32_t,
        std::string
    > TagsDictionaryRecordType;

    struct TagsObject {
        std::string         entity;
        std::string         arn;    // <prefix>:uuid
        Types::StringVec    entries;
    };

    typedef Poco::Tuple<
        std::string,
        std::string,
        std::string
    > TagsObjectRecordType;

    class TagsDictionaryDB : public ORM::DB<TagsDictionaryRecordType, TagsDictionary> {
        public:
            TagsDictionaryDB( ORM::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L);
        private:
    };

    class TagsObjectDB : public ORM::DB<TagsObjectRecordType, TagsObject> {
        public:
            TagsObjectDB( ORM::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L);
        private:
    };
}

#endif //OWPROV_STORAGE_TAGS_H
