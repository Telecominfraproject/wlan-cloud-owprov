//
// Created by stephane bourque on 2023-10-02.
//

#pragma once
#include <vector>
#include <utility>
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

        void ParseCertChain(const std::string &Chain, std::vector<std::string> &ChainVec) {
            std::istringstream os(Chain);
            std::string CurrentCert;
            bool InCert = false;
            std::string Line;
            while(std::getline(os,Line)) {
                if(Line=="-----BEGIN CERTIFICATE-----") {
                    InCert = true;
                    CurrentCert += Line;
                    CurrentCert += "\n";
                    continue;
                }
                if(Line=="-----END CERTIFICATE-----" && InCert) {
                    InCert = false;
                    CurrentCert += Line;
                    CurrentCert += "\n";
                    ChainVec.emplace_back(CurrentCert);
                    continue;
                }
                if(InCert) {
                    CurrentCert += Line;
                    CurrentCert += "\n";
                }
            }
        }

        void UpdateRadiusServerEntry( GWObjects::RadiusProxyServerConfig &Config,
                                      const ProvObjects::RADIUSEndPoint &Endpoint,
                                      const std::vector<ProvObjects::RADIUSServer> &Servers) {
            Config.monitor = false;
            Config.strategy = Endpoint.PoolStrategy;
            Config.monitorMethod = "none";
            Config.strategy = "random";
            for (const auto &Server: Servers) {
                GWObjects::RadiusProxyServerEntry PE;
                PE.radsec = false;
                PE.name = Server.Hostname;
                PE.ignore = false;
                PE.ip = Server.IP;
                PE.port = PE.radsecPort = Server.Port;
                PE.allowSelfSigned = false;
                PE.weight = 10;
                PE.secret = PE.radsecSecret = "radsec";
                Config.servers.emplace_back(PE);
            }
        }

        inline bool UpdateEndpoints( RESTAPIHandler *Client, std::uint64_t & ErrorCode,
                                     std::string & ErrorDetails,
                                     std::string & ErrorDescription) {

            std::vector<ProvObjects::RADIUSEndPoint>    Endpoints;
            GWObjects::RadiusProxyPoolList  Pools;
            StorageService()->RadiusEndpointDB().GetRecords(0,500,Endpoints);

            for(const auto &Endpoint:Endpoints) {
                GWObjects::RadiusProxyPool  PP;

                PP.name = Endpoint.info.name;
                PP.description = Endpoint.info.description;
                PP.useByDefault = false;
                PP.poolProxyIp = Endpoint.Index;
                PP.radsecKeepAlive = 25;
                PP.enabled = true;

                if(Endpoint.Type=="orion" && !Endpoint.RadsecServers.empty()) {
                    auto Svrs = OpenRoaming_Orion()->GetServers();
                    PP.radsecPoolType="orion";
                    ProvObjects::GooglOrionAccountInfo  OA;
                    if(StorageService()->OrionAccountsDB().GetRecord("id", Endpoint.RadsecServers[0].UseOpenRoamingAccount, OA)) {
                        for(auto *ServerType:{&PP.authConfig, &PP.acctConfig, &PP.coaConfig}) {
                            ServerType->monitor = false;
                            ServerType->strategy = Endpoint.PoolStrategy;
                            ServerType->monitorMethod = "none";
                            ServerType->strategy = "random";
                            int i=1;
                            for (const auto &Server: Svrs) {
                                GWObjects::RadiusProxyServerEntry PE;
                                PE.radsecCert = Utils::base64encode((const u_char *)OA.certificate.c_str(),OA.certificate.size());
                                PE.radsecKey = Utils::base64encode((const u_char *)OA.privateKey.c_str(),OA.privateKey.size());
                                for(const auto &cert:OA.cacerts) {
                                    auto C = Utils::base64encode((const u_char *)cert.c_str(),cert.size());
                                    PE.radsecCacerts.emplace_back(C);
                                }
                                PE.radsec = true;
                                PE.name = fmt::format("Server {}",i++);
                                PE.ignore = false;
                                PE.ip = Server.Hostname;
                                PE.port = PE.radsecPort = Server.Port;
                                PE.allowSelfSigned = false;
                                PE.weight = 10;
                                PE.secret = PE.radsecSecret = "radsec";
                                ServerType->servers.emplace_back(PE);
                            }
                        }
                        Pools.pools.emplace_back(PP);
                    }
                } else if(Endpoint.Type=="globalreach" && !Endpoint.RadsecServers.empty()) {
                    auto Svrs = OpenRoaming_GlobalReach()->GetServers();
                    PP.radsecPoolType="globalreach";
                    ProvObjects::GLBLRCertificateInfo   GRCertificate;
                    ProvObjects::GLBLRAccountInfo       GRAccountInfo;
                    if( StorageService()->GLBLRCertsDB().GetRecord("id",Endpoint.RadsecServers[0].UseOpenRoamingAccount,GRCertificate) &&
                        StorageService()->GLBLRAccountInfoDB().GetRecord("id",GRCertificate.accountId,GRAccountInfo)) {
                        for(auto *ServerType:{&PP.authConfig, &PP.acctConfig, &PP.coaConfig}) {
                            ServerType->monitor = false;
                            ServerType->monitorMethod = "none";
                            ServerType->strategy = Endpoint.PoolStrategy;
                            ServerType->strategy = "random";
                            int i = 1;
                            for (const auto &Server: Svrs) {
                                GWObjects::RadiusProxyServerEntry PE;
                                PE.radsecCert = Utils::base64encode((const u_char *)GRCertificate.certificate.c_str(),GRCertificate.certificate.size());
                                PE.radsecKey = Utils::base64encode((const u_char *)GRAccountInfo.CSRPrivateKey.c_str(),GRAccountInfo.CSRPrivateKey.size());
                                std::vector<std::string> Chain;
                                ParseCertChain(GRCertificate.certificateChain,Chain);
                                for(const auto &cert:Chain) {
                                    PE.radsecCacerts.emplace_back( Utils::base64encode((const u_char *)cert.c_str(),cert.size()));
                                }
                                PE.radsec = true;
                                PE.name = fmt::format("Server {}", i++);
                                PE.ignore = false;
                                PE.ip = Server.Hostname;
                                PE.port = PE.radsecPort = Server.Port;
                                PE.allowSelfSigned = false;
                                PE.weight = 10;
                                PE.secret = PE.radsecSecret = "radsec";
                                ServerType->servers.emplace_back(PE);
                            }
                        }
                        Pools.pools.emplace_back(PP);
                    }
                } else if(Endpoint.Type=="radsec"  && !Endpoint.RadsecServers.empty()) {
                    PP.radsecPoolType="radsec";
                    for(auto *ServerType:{&PP.authConfig, &PP.acctConfig, &PP.coaConfig}) {
                        ServerType->monitor = false;
                        ServerType->strategy = Endpoint.PoolStrategy;
                        ServerType->monitorMethod = "none";
                        ServerType->strategy = "random";
                        for (const auto &Server: Endpoint.RadsecServers) {
                            GWObjects::RadiusProxyServerEntry PE;
                            PE.radsecCert = Utils::base64encode((const u_char *)Server.Certificate.c_str(), Server.Certificate.size());
                            PE.radsecKey = Utils::base64encode((const u_char *)Server.PrivateKey.c_str(),Server.PrivateKey.size());
                            for(const auto &C:Server.CaCerts) {
                                PE.radsecCacerts.emplace_back(Utils::base64encode(
                                        (const u_char *) C.c_str(),
                                        C.size()));
                            }
                            PE.radsec = true;
                            PE.name = Server.Hostname;
                            PE.ignore = false;
                            PE.ip = Server.IP;
                            PE.port = PE.radsecPort = Server.Port;
                            PE.allowSelfSigned = false;
                            PE.weight = 10;
                            PE.secret = PE.radsecSecret = "radsec";
                            ServerType->servers.emplace_back(PE);
                        }
                    }
                    Pools.pools.emplace_back(PP);
                } else if(Endpoint.Type=="generic"  && !Endpoint.RadiusServers.empty()) {
                    PP.radsecPoolType="generic";
                    UpdateRadiusServerEntry(PP.authConfig, Endpoint, Endpoint.RadiusServers[0].Authentication);
                    UpdateRadiusServerEntry(PP.acctConfig, Endpoint, Endpoint.RadiusServers[0].Accounting);
                    UpdateRadiusServerEntry(PP.coaConfig, Endpoint, Endpoint.RadiusServers[0].CoA);
                    Pools.pools.emplace_back(PP);
                }
            }

/*
            Poco::JSON::Object  oo;
            Pools.to_json(oo);
            oo.stringify(std::cout,2,2);
*/
            GWObjects::RadiusProxyPoolList  NewPools;
            Poco::JSON::Object ErrorObj;
            if(SDK::GW::RADIUS::SetConfiguration(Client, Pools, NewPools, ErrorObj)) {
                ProvObjects::RADIUSEndpointUpdateStatus Status;
                Status.Read();
                Status.lastConfigurationChange = Status.lastUpdate = Utils::Now();
                return Status.Save();
            }
/*
            ErrorCode:
            type: integer
            ErrorDetails:
            type: string
            ErrorDescription:
            type: string
  */
            if(ErrorObj.has("ErrorCode") && !ErrorObj.isNull("ErrorCode"))
                ErrorCode = ErrorObj.get("ErrorCode");
            if(ErrorObj.has("ErrorDescription") && !ErrorObj.isNull("ErrorDescription"))
                ErrorDescription = ErrorObj.get("ErrorDescription").toString();
            if(ErrorObj.has("ErrorDetails") && !ErrorObj.isNull("ErrorDetails"))
                ErrorDetails += ErrorObj.get("ErrorDetails").toString();
            return false;
        }

    private:

    };



} // OpenWifi
