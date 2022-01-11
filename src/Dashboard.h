//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#pragma once

#include "framework/OpenWifiTypes.h"
#include "RESTObjects/RESTAPI_ProvObjects.h"

namespace OpenWifi {
	class ProvisioningDashboard {
	  public:
			void Create();
			[[nodiscard]] const ProvObjects::Report & Report() const { return DB_;}
			inline void Reset() { LastRun_=0; DB_.reset(); }
	  private:
	        ProvObjects::Report  	DB_{};
			uint64_t 				LastRun_=0;
	};
}
