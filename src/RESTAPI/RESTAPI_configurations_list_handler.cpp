//
// Created by stephane bourque on 2021-08-29.
//

#include "RESTAPI_configurations_list_handler.h"

#include "RESTAPI_db_helpers.h"
#include "RESTObjects/RESTAPI_ProvObjects.h"
#include "StorageService.h"

namespace OpenWifi {
	void RESTAPI_configurations_list_handler::DoGet() {
		return ListHandler<ConfigurationDB>("configurations", DB_, *this);
	}
} // namespace OpenWifi