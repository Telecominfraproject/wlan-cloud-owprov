//
// Created by stephane bourque on 2022-02-21.
//

#pragma once

#include "framework/orm.h"
#include "RESTObjects/RESTAPI_ProvObjects.h"

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
            uint64_t,
            uint64_t
    > SignupDBRecordType;

    class SignupDB : public ORM::DB<SignupDBRecordType, ProvObjects::SignupEntry> {
    public:
        SignupDB( OpenWifi::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L);
    private:
    };
}