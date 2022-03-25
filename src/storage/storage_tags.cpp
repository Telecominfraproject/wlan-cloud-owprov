//
// Created by stephane bourque on 2021-10-02.
//

#include "storage_tags.h"
#include "framework/OpenWifiTypes.h"
#include "framework/MicroService.h"
#include "StorageService.h"
#include <functional>

namespace OpenWifi {

    static  ORM::FieldVec    TagsDictionary_Fields{
        // object info
        ORM::Field{"entity",ORM::FieldType::FT_TEXT, 64},
        ORM::Field{"id",ORM::FieldType::FT_INT, 0,true},
        ORM::Field{"name",ORM::FieldType::FT_TEXT, 32}
    };

    static  ORM::IndexVec    TagsDictionaryDB_Indexes{
        { std::string("tags_dictionary_name_index"),
          ORM::IndexEntryVec{
            {std::string("name"),
             ORM::Indextype::ASC} } }
    };

    TagsDictionaryDB::TagsDictionaryDB( OpenWifi::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L) :
        DB(T, "TagsDictionary", TagsDictionary_Fields, TagsDictionaryDB_Indexes, P, L, "tgd") {}

    static  ORM::FieldVec    TagsObject_Fields{
        // object info
        ORM::Field{"entity",ORM::FieldType::FT_TEXT, 64},
        ORM::Field{"arn",ORM::FieldType::FT_TEXT, 64,true},      // ARN:uuid
        ORM::Field{"entries",ORM::FieldType::FT_TEXT}
    };

    static  ORM::IndexVec    TagsObject_Indexes{};

    TagsObjectDB::TagsObjectDB( OpenWifi::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L) :
        DB(T, "TagsObject", TagsObject_Fields, TagsObject_Indexes, P, L, "tag") {}
}

template<> void ORM::DB<    OpenWifi::TagsDictionaryRecordType, OpenWifi::TagsDictionary>::Convert(const OpenWifi::TagsDictionaryRecordType &In, OpenWifi::TagsDictionary &Out) {
    Out.entity = In.get<0>();
    Out.id = In.get<1>();
    Out.name = In.get<2>();
}

template<> void ORM::DB<    OpenWifi::TagsDictionaryRecordType, OpenWifi::TagsDictionary>::Convert(const OpenWifi::TagsDictionary &In, OpenWifi::TagsDictionaryRecordType &Out) {
    Out.set<0>(In.entity);
    Out.set<1>(In.id);
    Out.set<2>(In.name);
}

template<> void ORM::DB<    OpenWifi::TagsObjectRecordType, OpenWifi::TagsObject>::Convert(const OpenWifi::TagsObjectRecordType &In, OpenWifi::TagsObject &Out) {
    Out.entity = In.get <0>();
    Out.arn= In.get<1>();
    Out.entries = OpenWifi::RESTAPI_utils::to_object_array(In.get<2>());
}

template<> void ORM::DB<    OpenWifi::TagsObjectRecordType, OpenWifi::TagsObject>::Convert(const OpenWifi::TagsObject &In, OpenWifi::TagsObjectRecordType &Out) {
    Out.set<0>(In.entity);
    Out.set<1>(In.arn);
    Out.set<2>(OpenWifi::RESTAPI_utils::to_string(In.entries));
}
