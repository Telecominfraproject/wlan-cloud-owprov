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

        enum class PoolStrategy {
            round_robbin, random, weighted, unknown
        };

        enum class EndpointType {
            generic, radsec, globalreach, orion, unknown
        };

        static inline EndpointType EndpointType(const std::string &T) {
            if(T=="generic") return EndpointType::generic;
            if(T=="radsec") return EndpointType::radsec;
            if(T=="globalreach") return EndpointType::globalreach;
            if(T=="orion") return EndpointType::orion;
            return EndpointType::unknown;
        }

        static inline PoolStrategy PoolStrategy(const std::string &T) {
            if(T=="round_robbin") return PoolStrategy::round_robbin;
            if(T=="random") return PoolStrategy::random;
            if(T=="weighted") return PoolStrategy::weighted;
            return PoolStrategy::unknown;
        }

        static inline std::string to_string(enum EndpointType T) {
            switch(T) {
                case EndpointType::generic: return "generic";
                case EndpointType::radsec: return "radsec";
                case EndpointType::globalreach: return "globalreach";
                case EndpointType::orion: return "orion";
                default:
                    return "unknown";
            }
        }

        static inline std::string to_string(enum PoolStrategy T) {
            switch(T) {
                case PoolStrategy::round_robbin: return "round_robbin";
                case PoolStrategy::random: return "random";
                case PoolStrategy::weighted: return "weighted";
                default:
                    return "unknown";
            }
        }

        static inline bool ValidIndex(const std::string &I) {
            static uint32_t Low = Utils::IPtoInt("0.0.1.1");
            static uint32_t High = Utils::IPtoInt("0.0.2.254");
            auto IP = Utils::IPtoInt(I);
            return (IP>=Low) && (IP<=High);
        }

    private:

    };
} // namespace OpenWifi
