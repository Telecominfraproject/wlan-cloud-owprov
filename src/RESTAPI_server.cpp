//
// Created by stephane bourque on 2021-05-09.
//

#include "Poco/URI.h"

#include "RESTAPI_server.h"
#include "Utils.h"
#include "RESTAPI_handler.h"

#include "RESTAPI_system_command.h"

#include "RESTAPI_entity_handler.h"
#include "RESTAPI_contact_handler.h"
#include "RESTAPI_location_handler.h"
#include "RESTAPI_venue_handler.h"
#include "RESTAPI_inventory_handler.h"
#include "RESTAPI_managementPolicy_handler.h"
#include "RESTAPI_inventory_list_handler.h"
#include "RESTAPI_entity_list_handler.h"

namespace uCentral {

    class RESTAPI_server *RESTAPI_server::instance_ = nullptr;

    RESTAPI_server::RESTAPI_server() noexcept:
    SubSystemServer("RESTAPIServer", "RESTAPIServer", "owprov.restapi")
    {
    }

    int RESTAPI_server::Start() {
        Logger_.information("Starting.");

        for(const auto & Svr: ConfigServersList_) {
            Logger_.information(Poco::format("Starting: %s:%s Keyfile:%s CertFile: %s", Svr.Address(), std::to_string(Svr.Port()),
                                             Svr.KeyFile(),Svr.CertFile()));

            auto Sock{Svr.CreateSecureSocket(Logger_)};

            Svr.LogCert(Logger_);
            if(!Svr.RootCA().empty())
                Svr.LogCas(Logger_);

            auto Params = new Poco::Net::HTTPServerParams;
            Params->setMaxThreads(50);
            Params->setMaxQueued(200);
            Params->setKeepAlive(true);
            uint64_t T = 45000;
            Params->setKeepAliveTimeout(T);
            Params->setMaxKeepAliveRequests(200);
            Params->setTimeout(T + 10000);

            auto NewServer = std::make_unique<Poco::Net::HTTPServer>(new RequestHandlerFactory, Pool_, Sock, Params);
            NewServer->start();
            RESTServers_.push_back(std::move(NewServer));
        }

        return 0;
    }

    Poco::Net::HTTPRequestHandler *RequestHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest & Request) {

        Logger_.debug(Poco::format("REQUEST(%s): %s %s", uCentral::Utils::FormatIPv6(Request.clientAddress().toString()), Request.getMethod(), Request.getURI()));

        Poco::URI uri(Request.getURI());
        auto *Path = uri.getPath().c_str();
        RESTAPIHandler::BindingMap Bindings;

        return  RESTAPI_Router<
                RESTAPI_system_command,
                OpenWifi::RESTAPI_entity_handler,
                OpenWifi::RESTAPI_entity_list_handler,
                OpenWifi::RESTAPI_contact_handler,
                OpenWifi::RESTAPI_location_handler,
                OpenWifi::RESTAPI_venue_handler,
                OpenWifi::RESTAPI_inventory_handler,
                OpenWifi::RESTAPI_inventory_list_handler,
                OpenWifi::RESTAPI_managementPolicy_handler
                >(Path,Bindings,Logger_);
    }

    void RESTAPI_server::Stop() {
        Logger_.information("Stopping ");
        for( const auto & svr : RESTServers_ )
            svr->stop();
    }

}  // namespace