//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//
#ifndef UCENTRALGW_DASHBOARD_H
#define UCENTRALGW_DASHBOARD_H

#include "framework/OpenWifiTypes.h"
#include "RESTAPI/RESTAPI_ProvObjects.h"

namespace OpenWifi {
	class TopoDashboard {
	  public:
			void Create();
			[[nodiscard]] const ProvObjects::Report & Report() const { return DB_;}
			inline void Reset() { LastRun_=0; DB_.reset(); }
	  private:
	        ProvObjects::Report  	DB_{};
			uint64_t 				LastRun_=0;
	};
}

#endif // UCENTRALGW_DASHBOARD_H
