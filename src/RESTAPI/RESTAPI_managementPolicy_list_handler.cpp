//
// Created by stephane bourque on 2021-08-26.
//

#include "RESTAPI_managementPolicy_list_handler.h"

#include "RESTAPI/RESTAPI_db_helpers.h"
#include "RESTObjects/RESTAPI_ProvObjects.h"
#include "StorageService.h"

namespace OpenWifi {
	void RESTAPI_managementPolicy_list_handler::DoGet() {
		return ListHandler<PolicyDB>("managementPolicies", DB_, *this);
	}
} // namespace OpenWifi