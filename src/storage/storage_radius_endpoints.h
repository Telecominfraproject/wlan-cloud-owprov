//
// Created by stephane bourque on 2023-09-27.
//

#pragma once

#include "RESTObjects/RESTAPI_ProvObjects.h"
#include "framework/orm.h"

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
            bool,
            std::string,
            std::uint64_t
    >   RadiusEndpointDbRecordType;

    class RadiusEndpointDB : public ORM::DB<RadiusEndpointDbRecordType, ProvObjects::RADIUSEndPoint> {
    public:
        RadiusEndpointDB(OpenWifi::DBType T, Poco::Data::SessionPool &P, Poco::Logger &L);
        virtual ~RadiusEndpointDB(){};
        bool Upgrade(uint32_t from, uint32_t &to) override;

        static inline bool ValidIndex(const std::string &I) {
            static uint32_t Low = Utils::IPtoInt("0.0.1.1");
            static uint32_t High = Utils::IPtoInt("0.0.2.254");
            auto IP = Utils::IPtoInt(I);
            return (IP>=Low) && (IP<=High);
        }

    private:

    };
} // namespace OpenWifi
