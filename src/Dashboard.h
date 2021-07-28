//
// Created by stephane bourque on 2021-07-21.
//

#ifndef UCENTRALGW_DASHBOARD_H
#define UCENTRALGW_DASHBOARD_H

#include "uCentralTypes.h"
#include "RESTAPI_TopoObjects.h"

namespace uCentral {
	class TopoDashboard {
	  public:
			void Create();
			const TopoObjects::Report & Report() const { return DB_;}
			inline void Reset() { LastRun_=0; DB_.reset(); }
	  private:
            TopoObjects::Report  	DB_;
			uint64_t 				LastRun_=0;
	};
}

#endif // UCENTRALGW_DASHBOARD_H
