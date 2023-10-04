//
// Created by stephane bourque on 2023-10-02.
//

#pragma once
#include <framework/AppServiceRegistry.h>
#include <framework/utils.h>
#include <StorageService.h>

namespace OpenWifi {
    class RadiusEndpointUpdater {
    public:
        inline bool UpdateEndpoints( [[maybe_unused]] std::string & Error,
                                     [[maybe_unused]] uint64_t &ErrorNum  ) {

            std::vector<ProvObjects::RADIUSEndPoint>    Endpoints;
            StorageService()->RadiusEndpointDB().GetRecords(0,500,Endpoints);
            Poco::JSON::Array   RadiusPools;

            for(const auto &Endpoint:Endpoints) {
                Poco::JSON::Object  PoolEntry;
                PoolEntry.set("description", Endpoint.info.description);
                PoolEntry.set("name", Endpoint.info.name);
                PoolEntry.set("useByDefault", false);
                PoolEntry.set("poolProxyIp", Endpoint.Index);
                PoolEntry.set("radsecPoolKeepAlive",25);
                if(Endpoint.Type=="orion") {
                    PoolEntry.set("radsecPoolType","orion");
                } else if(Endpoint.Type=="radsec") {
                    PoolEntry.set("radsecPoolType","radsec");
                } else if(Endpoint.Type=="radius") {
                    PoolEntry.set("radsecPoolType","generic");
                } else if(Endpoint.Type=="globalreach") {
                    PoolEntry.set("radsecPoolType","globalreach");
                }
                RadiusPools.add(PoolEntry);
            }
            Poco::JSON::Object  RadiusConfig;
            RadiusConfig.set("pools", RadiusPools);

            AppServiceRegistry().Set("radiusEndpointLastUpdate", Utils::Now());
            return false;
        }
    private:

    };
} // OpenWifi
