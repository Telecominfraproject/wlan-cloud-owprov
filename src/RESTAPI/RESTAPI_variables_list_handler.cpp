//
// Created by stephane bourque on 2022-02-23.
//

#include "RESTAPI_variables_list_handler.h"
#include "RESTAPI/RESTAPI_db_helpers.h"

namespace OpenWifi {

	void RESTAPI_variables_list_handler::DoGet() {
		auto templateTag = GetParameter("templateTag","");
        if(templateTag.empty()) {		
			return ListHandler<VariablesDB>("variableBlocks", DB_, *this);
		}
        VariablesDB::RecordVec Variables;
        auto Where = fmt::format(" templateTag = '{}' ", templateTag);

		//poco_debug(
		//	Logger(),
		//	fmt::format("RESTAPI_variables_list_handler Where clause {}", Where));

        DB_.GetRecords(QB_.Offset, QB_.Limit, Variables, Where, " ORDER BY name ");
        return ReturnObject("variableBlocks",Variables);		
	}

} // namespace OpenWifi