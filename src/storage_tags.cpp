//
// Created by stephane bourque on 2021-10-02.
//

#include "storage_tags.h"
#include "OpenWifiTypes.h"
#include "RESTAPI_utils.h"
#include "StorageService.h"
#include <functional>

namespace OpenWifi {

    static  ORM::FieldVec    TagsDictionary_Fields{
        // object info
        ORM::Field{"id",ORM::FieldType::FT_INT, 0,true},
        ORM::Field{"name",ORM::FieldType::FT_TEXT, 32, }
    };

    static  ORM::IndexVec    TagsDictionaryDB_Indexes{
        { std::string("tags_dictionary_name_index"),
          ORM::IndexEntryVec{
            {std::string("name"),
             ORM::Indextype::ASC} } }
    };

    TagsDictionaryDB::TagsDictionaryDB( ORM::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L) :
    DB(T, "tagsdictionary", TagsDictionary_Fields, TagsDictionaryDB_Indexes, P, L, "tgd") {}

}

template<> void ORM::DB<    OpenWifi::TagsDictionaryRecordType, OpenWifi::TagsDictionary>::Convert(OpenWifi::TagsDictionaryRecordType &In, OpenWifi::TagsDictionary &Out) {
    Out.id = In.get<0>();
    Out.name = In.get<1>();
}

template<> void ORM::DB<    OpenWifi::TagsDictionaryRecordType, OpenWifi::TagsDictionary>::Convert(OpenWifi::TagsDictionary &In, OpenWifi::TagsDictionaryRecordType &Out) {
    Out.set<0>(In.id);
    Out.set<1>(In.name);
}
