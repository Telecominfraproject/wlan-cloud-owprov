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
            bool
    >   RadiusEndpointDbRecordType;

    class RadiusEndpointDB : public ORM::DB<RadiusEndpointDbRecordType, ProvObjects::RADIUSEndPoint> {
    public:
        RadiusEndpointDB(OpenWifi::DBType T, Poco::Data::SessionPool &P, Poco::Logger &L);
        virtual ~RadiusEndpointDB(){};
        bool Upgrade(uint32_t from, uint32_t &to) override;

        enum class PoolStrategy {
            none, random, weighted, unknown
        };

        enum class EndpointType {
            radius, radsec, globalreach, orion, unknown
        };

        static inline EndpointType EndpointType(const std::string &T) {
            if(T=="radius") return EndpointType::radius;
            if(T=="radsec") return EndpointType::radsec;
            if(T=="globalreach") return EndpointType::globalreach;
            if(T=="orion") return EndpointType::orion;
            return EndpointType::unknown;
        }

        static inline PoolStrategy PoolStrategy(const std::string &T) {
            if(T=="none") return PoolStrategy::none;
            if(T=="random") return PoolStrategy::random;
            if(T=="weighted") return PoolStrategy::weighted;
            return PoolStrategy::unknown;
        }

        static inline std::string to_string(enum EndpointType T) {
            switch(T) {
                case EndpointType::radius: return "radius";
                case EndpointType::radsec: return "radsec";
                case EndpointType::globalreach: return "globalreach";
                case EndpointType::orion: return "orion";
                default:
                    return "unknown";
            }
        }

        static inline std::string to_string(enum PoolStrategy T) {
            switch(T) {
                case PoolStrategy::none: return "none";
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
