//
// Created by stephane bourque on 2023-10-02.
//

#pragma once
#include <framework/AppServiceRegistry.h>
#include <framework/utils.h>
#include <StorageService.h>
#include <RadiusEndpointTypes/OrionWifi.h>
#include <RadiusEndpointTypes/GlobalReach.h>

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
                    auto Servers = OpenRoaming_Orion()->GetServers();
                    Poco::JSON::Object ServerDetails;
                    ServerDetails.set("methodParameters", Poco::JSON::Array() );
                    ServerDetails.set("monitor", false );
                    ServerDetails.set("monitorMethod", "none" );
                    ServerDetails.set("strategy","random");
                    Poco::JSON::Array   ServerArray;
                    ProvObjects::GooglOrionAccountInfo  OA;
                    StorageService()->OrionAccountsDB().GetRecord("id",Endpoint.RadsecServers[0].UseOpenRoamingAccount,OA);
                    int i=1;
                    for(const auto &Server:Servers) {
                        Poco::JSON::Object  AuthConfig;
                        AuthConfig.set("allowSelfSigned", false);
                        AuthConfig.set("ignore", false);
                        AuthConfig.set("name", fmt::format("Server {}",i));
                        AuthConfig.set("ip", Server.Hostname);
                        AuthConfig.set("radsecPort", Server.Port);
                        AuthConfig.set("radsecCert", Utils::base64encode((const u_char *)OA.certificate.c_str(),OA.certificate.size()));
                        AuthConfig.set("radsecKey", Utils::base64encode((const u_char *)OA.privateKey.c_str(),OA.privateKey.size()));
                        Poco::JSON::Array   CaCerts;
                        for(const auto &cert:OA.cacerts) {
                            CaCerts.add(Utils::base64encode((const u_char *)cert.c_str(),cert.size()));
                        }
                        AuthConfig.set("radsecCacerts", CaCerts);
                        AuthConfig.set("radsecSecret","radsec");
                        i++;
                        ServerArray.add(AuthConfig);
                    }
                    ServerDetails.set("servers",ServerArray);
                    RadiusPools.add(ServerDetails);
                } else if(Endpoint.Type=="globalreach") {
                    PoolEntry.set("radsecPoolType","globalreach");
                    auto Servers = OpenRoaming_GlobalReach()->GetServers();
                    Poco::JSON::Object ServerDetails;
                    ServerDetails.set("methodParameters", Poco::JSON::Array() );
                    ServerDetails.set("monitor", false );
                    ServerDetails.set("monitorMethod", "none" );
                    ServerDetails.set("strategy","random");
                    Poco::JSON::Array   ServerArray;
                    ProvObjects::GLBLRCertificateInfo   GRCertificate;
                    ProvObjects::GLBLRAccountInfo       GRAccountInfo;
                    StorageService()->GLBLRCertsDB().GetRecord("id",Endpoint.RadsecServers[0].UseOpenRoamingAccount,GRCertificate);
                    StorageService()->GLBLRAccountInfoDB().GetRecord("id",GRCertificate.accountId,GRAccountInfo);
                    int i=1;
                    for(const auto &Server:Servers) {
                        Poco::JSON::Object  AuthConfig;
                        AuthConfig.set("allowSelfSigned", false);
                        AuthConfig.set("ignore", false);
                        AuthConfig.set("name", fmt::format("Server {}",i));
                        AuthConfig.set("ip", Server.Hostname);
                        AuthConfig.set("radsecPort", Server.Port);
                        AuthConfig.set("radsecCert", Utils::base64encode((const u_char *)GRCertificate.certificate.c_str(),GRCertificate.certificate.size()));
                        AuthConfig.set("radsecKey", Utils::base64encode((const u_char *)GRAccountInfo.CSRPrivateKey.c_str(),GRAccountInfo.CSRPrivateKey.size()));
                        Poco::JSON::Array   CaCerts;
                        for(const auto &cert:GRCertificate.certificateChain) {
                            CaCerts.add(Utils::base64encode((const u_char *)cert.c_str(),cert.size()));
                        }
                        AuthConfig.set("radsecCacerts", CaCerts);
                        AuthConfig.set("radsecSecret","radsec");
                        i++;
                        ServerArray.add(AuthConfig);
                    }
                    ServerDetails.set("servers",ServerArray);
                    RadiusPools.add(ServerDetails);
                } else if(Endpoint.Type=="radius") {
                    PoolEntry.set("radsecPoolType","generic");
                    auto Servers = OpenRoaming_GlobalReach()->GetServers();
                    for(const auto &Server:Servers) {

                    }
                } else if(Endpoint.Type=="radsec") {
                    PoolEntry.set("radsecPoolType", "radsec");
                    for (const auto &Server: Endpoint.RadsecServers) {

                    }
                }
                RadiusPools.add(PoolEntry);
            }

            Poco::JSON::Object  RadiusConfig;
            RadiusConfig.set("pools", RadiusPools);
            RadiusConfig.stringify(std::cout,4,2);

            AppServiceRegistry().Set("radiusEndpointLastUpdate", Utils::Now());
            return false;
        }
    private:

    };
} // OpenWifi
