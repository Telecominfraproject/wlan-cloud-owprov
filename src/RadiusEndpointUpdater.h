//
// Created by stephane bourque on 2023-10-02.
//

#pragma once
#include <framework/AppServiceRegistry.h>
#include <framework/utils.h>
#include <StorageService.h>
#include <RadiusEndpointTypes/OrionWifi.h>
#include <RadiusEndpointTypes/GlobalReach.h>
#include <sdks/SDK_gw.h>
#include <RESTObjects/RESTAPI_GWobjects.h>

namespace OpenWifi {
    class RadiusEndpointUpdater {
    public:

        void AddServers(const  std::vector<ProvObjects::RADIUSServer> &ServerList, Poco::JSON::Object &O) {
            O.set("methodParameters", Poco::JSON::Array());
            O.set("monitor" , false);
            O.set("monitorMethod", "none");
            O.set("strategy", "random");
            Poco::JSON::Array   ServerArray;
            for(const auto &server:ServerList) {
                Poco::JSON::Object  InnerServer;
                InnerServer.set("allowSelfSigned", false);
                InnerServer.set("certificate", "");
                InnerServer.set("ignore", false);
                InnerServer.set("ip",server.IP);
                InnerServer.set("port", server.Port);
                InnerServer.set("secret", server.Secret);
                InnerServer.set("name", server.Hostname);
                InnerServer.set("radsec", false);
                ServerArray.add(InnerServer);
            }
            O.set("servers", ServerArray);
        }

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
                    Poco::JSON::Object AuthConfig;
                    AuthConfig.set("methodParameters", Poco::JSON::Array() );
                    AuthConfig.set("monitor", false );
                    AuthConfig.set("monitorMethod", "none" );
                    AuthConfig.set("strategy","random");
                    Poco::JSON::Array   ServerArray;
                    ProvObjects::GooglOrionAccountInfo  OA;
                    StorageService()->OrionAccountsDB().GetRecord("id",Endpoint.RadsecServers[0].UseOpenRoamingAccount,OA);
                    int i=1;
                    for(const auto &Server:Servers) {
                        Poco::JSON::Object  InnerServer;
                        InnerServer.set("allowSelfSigned", false);
                        InnerServer.set("ignore", false);
                        InnerServer.set("name", fmt::format("Server {}",i));
                        InnerServer.set("ip", Server.Hostname);
                        InnerServer.set("radsecPort", Server.Port);
                        InnerServer.set("radsec", true);
                        InnerServer.set("radsecCert", Utils::base64encode((const u_char *)OA.certificate.c_str(),OA.certificate.size()));
                        InnerServer.set("radsecKey", Utils::base64encode((const u_char *)OA.privateKey.c_str(),OA.privateKey.size()));
                        Poco::JSON::Array   CaCerts;
                        for(const auto &cert:OA.cacerts) {
                            CaCerts.add(Utils::base64encode((const u_char *)cert.c_str(),cert.size()));
                        }
                        InnerServer.set("radsecCacerts", CaCerts);
                        InnerServer.set("radsecSecret","radsec");
                        i++;
                        ServerArray.add(InnerServer);
                    }
                    AuthConfig.set("servers",ServerArray);
                    PoolEntry.set("authConfig", AuthConfig);
                    RadiusPools.add(PoolEntry);
                } else if(Endpoint.Type=="globalreach") {
                    PoolEntry.set("radsecPoolType","globalreach");
                    auto Servers = OpenRoaming_GlobalReach()->GetServers();
                    Poco::JSON::Object AuthConfig;
                    AuthConfig.set("methodParameters", Poco::JSON::Array() );
                    AuthConfig.set("monitor", false );
                    AuthConfig.set("monitorMethod", "none" );
                    AuthConfig.set("strategy","random");
                    Poco::JSON::Array   ServerArray;
                    ProvObjects::GLBLRCertificateInfo   GRCertificate;
                    ProvObjects::GLBLRAccountInfo       GRAccountInfo;
                    StorageService()->GLBLRCertsDB().GetRecord("id",Endpoint.RadsecServers[0].UseOpenRoamingAccount,GRCertificate);
                    StorageService()->GLBLRAccountInfoDB().GetRecord("id",GRCertificate.accountId,GRAccountInfo);
                    int i=1;
                    for(const auto &Server:Servers) {
                        Poco::JSON::Object  InnerServer;
                        InnerServer.set("allowSelfSigned", false);
                        InnerServer.set("ignore", false);
                        InnerServer.set("name", fmt::format("Server {}",i));
                        InnerServer.set("ip", Server.Hostname);
                        InnerServer.set("radsec", true);
                        InnerServer.set("radsecPort", Server.Port);
                        InnerServer.set("radsecCert", Utils::base64encode((const u_char *)GRCertificate.certificate.c_str(),GRCertificate.certificate.size()));
                        InnerServer.set("radsecKey", Utils::base64encode((const u_char *)GRAccountInfo.CSRPrivateKey.c_str(),GRAccountInfo.CSRPrivateKey.size()));
                        InnerServer.set("radsecCacerts", Utils::base64encode((const u_char *)GRCertificate.certificateChain.c_str(),GRCertificate.certificateChain.size()));
                        InnerServer.set("radsecSecret","radsec");
                        i++;
                        ServerArray.add(InnerServer);
                    }
                    AuthConfig.set("servers",ServerArray);
                    PoolEntry.set("authConfig", AuthConfig);
                    RadiusPools.add(PoolEntry);
                } else if(Endpoint.Type=="radsec") {
                    PoolEntry.set("radsecPoolType","generic");
                    Poco::JSON::Object AuthConfig;
                    AuthConfig.set("methodParameters", Poco::JSON::Array() );
                    AuthConfig.set("monitor", false );
                    AuthConfig.set("monitorMethod", "none" );
                    AuthConfig.set("strategy","random");
                    Poco::JSON::Array   ServerArray;
                    int i=1;
                    for(const auto &Server:Endpoint.RadsecServers) {
                        Poco::JSON::Object  InnerServer;
                        InnerServer.set("allowSelfSigned", false);
                        InnerServer.set("ignore", false);
                        InnerServer.set("name", fmt::format("Server {}",i));
                        InnerServer.set("ip", Server.Hostname);
                        InnerServer.set("radsec", true);
                        InnerServer.set("radsecPort", Server.Port);
                        InnerServer.set("radsecCert", Utils::base64encode((const u_char *)Server.Certificate.c_str(), Server.Certificate.size()));
                        InnerServer.set("radsecKey", Utils::base64encode((const u_char *)Server.PrivateKey.c_str(), Server.PrivateKey.size()));
                        Poco::JSON::Array   CertArray;
                        for(const auto & cert : Server.CaCerts) {
                            CertArray.add(Utils::base64encode((const u_char *)cert.c_str(), cert.size()));
                        }
                        InnerServer.set("radsecCacerts", CertArray);
                        InnerServer.set("radsecSecret","radsec");
                        i++;
                        ServerArray.add(InnerServer);
                    }
                    AuthConfig.set("servers",ServerArray);
                    PoolEntry.set("authConfig", AuthConfig);
                    RadiusPools.add(PoolEntry);
                } else if(Endpoint.Type=="radius") {
                    PoolEntry.set("radsecPoolType", "radius");
                    const auto &server = Endpoint.RadiusServers[0];
                    Poco::JSON::Object  ServerEntry;
                    Poco::JSON::Object  AcctConfig, AuthConfig, CoAConfig, InnerServer;
                    AddServers(server.Authentication,AuthConfig);
                    AddServers(server.Accounting,AcctConfig);
                    AddServers(server.CoA,CoAConfig);
                    PoolEntry.set("authConfig", AuthConfig);
                    PoolEntry.set("acctConfig", AcctConfig);
                    PoolEntry.set("coaConfig", CoAConfig);
                }
            }

            Poco::JSON::Object  RadiusConfig;
            RadiusConfig.set("pools", RadiusPools);
            RadiusConfig.stringify(std::cout,4,2);

            GWObjects::RadiusProxyPoolList  NewPools;
            if(SDK::GW::RADIUS::SetConfiguration(nullptr,RadiusConfig,NewPools)) {
                AppServiceRegistry().Set("radiusEndpointLastUpdate", Utils::Now());
                return true;
            }
            Error = "Could not update the controller.";
            ErrorNum = 1;

            return false;
        }
    private:

    };
} // OpenWifi
