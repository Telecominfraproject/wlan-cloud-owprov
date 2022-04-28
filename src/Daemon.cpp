//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "Poco/Util/Application.h"
#include "Poco/Util/Option.h"
#include "Poco/Environment.h"

#include "Daemon.h"
#include "StorageService.h"
#include "AutoDiscovery.h"
#include "framework/ConfigurationValidator.h"
#include "SerialNumberCache.h"
#include "JobController.h"
#include "FindCountry.h"
#include "Signup.h"
#include "DeviceTypeCache.h"
#include "FileDownloader.h"

namespace OpenWifi {
	class Daemon *Daemon::instance_ = nullptr;

	class Daemon *Daemon::instance() {
		if (instance_ == nullptr) {
			instance_ = new Daemon(vDAEMON_PROPERTIES_FILENAME,
								   vDAEMON_ROOT_ENV_VAR,
								   vDAEMON_CONFIG_ENV_VAR,
								   vDAEMON_APP_NAME,
								   vDAEMON_BUS_TIMER,
								   SubSystemVec{
									   OpenWifi::StorageService(),
                                       DeviceTypeCache(),
									   ConfigurationValidator(),
									   SerialNumberCache(),
									   AutoDiscovery(),
									   JobController(),
									   WebSocketClientServer(),
                                       FindCountryFromIP(),
                                       Signup(),
                                       FileDownloader()
								   });
		}
		return instance_;
	}

	void Daemon::PostInitialization([[maybe_unused]] Poco::Util::Application &self) {
	    if(MicroService::instance().ConfigGetBool("firmware.updater.upgrade",false)) {
	        if(MicroService::instance().ConfigGetBool("firmware.updater.releaseonly",false)) {
	            FWRules_ = ProvObjects::upgrade_release_only;
	        } else {
	            FWRules_ = ProvObjects::upgrade_latest;
	        }
	    } else {
	        FWRules_ = ProvObjects::dont_upgrade;
	    }

        WebSocketProcessor_ = std::make_unique<ProvWebSocketClient>(logger());

        AssetDir_ = MicroService::instance().DataDir() + "/wwwassets";
        Poco::File	DataDir(AssetDir_);
        if(!DataDir.exists()) {
            try {
                DataDir.createDirectory();
            } catch (const Poco::Exception &E) {
                logger().log(E);
            }
        }
    }
}

int main(int argc, char **argv) {
	try {
		auto App = OpenWifi::Daemon::instance();
		auto ExitCode =  App->run(argc, argv);
		delete App;

		return ExitCode;

	} catch (Poco::Exception &exc) {
		std::cerr << exc.displayText() << std::endl;
		return Poco::Util::Application::EXIT_SOFTWARE;
	}
}

// end of namespace