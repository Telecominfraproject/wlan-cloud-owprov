//
// Created by stephane bourque on 2022-11-03.
//

#pragma once

#include "framework/orm.h"
#include "RESTObjects/RESTAPI_ProvObjects.h"

namespace OpenWifi {

    typedef Poco::Tuple<
            std::string,
            std::string,
            std::string
    > OverridesDBRecordType;

    class OverridesDB : public ORM::DB<OverridesDBRecordType, ProvObjects::ConfigurationOverrideList> {
    public:
        explicit OverridesDB( OpenWifi::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L);
        virtual ~OverridesDB() {};
        inline uint32_t Version() override {
            return 1;
        }

        bool Upgrade(uint32_t from, uint32_t &to) override;

    private:
    };

} // OpenWifi

