//
// Created by stephane bourque on 2022-04-07.
//

#include "RESTAPI_op_contact_list_handler.h"
#include "RESTAPI_db_helpers.h"

namespace OpenWifi {
	void RESTAPI_op_contact_list_handler::DoGet() {
		auto operatorId = GetParameter("operatorId");

		if (operatorId.empty() || !StorageService()->OperatorDB().Exists("id", operatorId)) {
			return BadRequest(RESTAPI::Errors::OperatorIdMustExist);
		}
		return ListHandlerForOperator<OpContactDB>("contacts", DB_, *this, operatorId);
	}

} // namespace OpenWifi