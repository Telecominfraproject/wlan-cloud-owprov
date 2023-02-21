//
// Created by stephane bourque on 2021-08-23.
//

#include "RESTAPI_contact_list_handler.h"

#include "RESTAPI_db_helpers.h"
#include "RESTObjects/RESTAPI_ProvObjects.h"
#include "StorageService.h"

namespace OpenWifi {
	void RESTAPI_contact_list_handler::DoGet() {
		return ListHandler<ContactDB>("contacts", DB_, *this);
	}
} // namespace OpenWifi