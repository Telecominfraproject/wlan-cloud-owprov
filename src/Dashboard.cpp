//
// Created by stephane bourque on 2021-07-21.
//

#include "Dashboard.h"
#include "StorageService.h"

namespace uCentral {
	void TopoDashboard::Create() {
		uint64_t Now = std::time(nullptr);
		if(LastRun_==0 || (Now-LastRun_)>120) {
			DB_.reset();
			//  Todo: call dashboard creation code.
			LastRun_ = Now;
		}
	}
}
