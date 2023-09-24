//
// Created by stephane bourque on 2023-09-17.
//

#pragma once

#include "RESTObjects/RESTAPI_ProvObjects.h"
#include "framework/orm.h"

namespace OpenWifi {

    typedef Poco::Tuple<std::string,
            std::string,
            std::string,
            std::string,
            uint64_t,
            uint64_t,
            std::string,
            std::string,
            std::string>
            OrionAccountsDBRecordType;

    class OrionAccountsDB : public ORM::DB<OrionAccountsDBRecordType, ProvObjects::GooglOrionAccountInfo> {
    public:
        OrionAccountsDB(OpenWifi::DBType T, Poco::Data::SessionPool &P, Poco::Logger &L);
        virtual ~OrionAccountsDB(){};
        bool Upgrade(uint32_t from, uint32_t &to) override;
    private:

    };

} // namespace OpenWifi
