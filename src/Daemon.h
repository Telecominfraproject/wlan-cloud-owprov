//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#pragma once

#include <array>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <set>

#include "Dashboard.h"
#include "framework/MicroService.h"
#include "framework/MicroServiceNames.h"
#include "framework/OpenWifiTypes.h"
#include "RESTObjects/RESTAPI_ProvObjects.h"
#include "ProvWebSocketClient.h"

namespace OpenWifi {

	[[maybe_unused]] static const char * vDAEMON_PROPERTIES_FILENAME = "owprov.properties";
    [[maybe_unused]] static const char * vDAEMON_ROOT_ENV_VAR = "OWPROV_ROOT";
    [[maybe_unused]] static const char * vDAEMON_CONFIG_ENV_VAR = "OWPROV_CONFIG";
    [[maybe_unused]] static const char * vDAEMON_APP_NAME = uSERVICE_PROVISIONING.c_str() ;
    [[maybe_unused]] static const uint64_t vDAEMON_BUS_TIMER = 10000;

    class Daemon : public MicroService {
		public:
			explicit Daemon(const std::string & PropFile,
							const std::string & RootEnv,
							const std::string & ConfigEnv,
							const std::string & AppName,
						  	uint64_t 	BusTimer,
							const SubSystemVec & SubSystems) :
				MicroService( PropFile, RootEnv, ConfigEnv, AppName, BusTimer, SubSystems) {};

			static Daemon *instance();
			inline OpenWifi::ProvisioningDashboard & GetDashboard() { return DB_; }
			Poco::Logger & Log() { return Poco::Logger::get(AppName()); }
			ProvObjects::FIRMWARE_UPGRADE_RULES FirmwareRules() const { return FWRules_; }
            inline const std::string & AssetDir() { return AssetDir_; }
            void PostInitialization(Poco::Util::Application &self);

	  	private:
			static Daemon 				            *instance_;
			OpenWifi::ProvisioningDashboard		    DB_{};
			ProvObjects::FIRMWARE_UPGRADE_RULES     FWRules_{ProvObjects::dont_upgrade};
            std::string                             AssetDir_;
            std::unique_ptr<ProvWebSocketClient>    WebSocketProcessor_;
    };

	inline Daemon * Daemon() { return Daemon::instance(); }
    void DaemonPostInitialization(Poco::Util::Application &self);
}

