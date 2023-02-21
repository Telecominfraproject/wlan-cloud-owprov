//
// Created by stephane bourque on 2021-11-09.
//

#include "RESTAPI_map_list_handler.h"
#include "RESTAPI/RESTAPI_db_helpers.h"
#include "RESTObjects/RESTAPI_ProvObjects.h"
#include "StorageService.h"

namespace OpenWifi {
	void RESTAPI_map_list_handler::DoGet() {
		const char *BlockName{"list"};
		if (GetBoolParameter("myMaps", false)) {
			auto where = DB_.OP("creator", ORM::EQ, UserInfo_.userinfo.id);
			MapDB::RecordVec Maps;
			DB_.GetRecords(QB_.Offset, QB_.Limit, Maps, where);
			return MakeJSONObjectArray(BlockName, Maps, *this);
		} else if (GetBoolParameter("sharedWithMe", false)) {

		} else {
			return ListHandler<MapDB>(BlockName, DB_, *this);
		}
	}
} // namespace OpenWifi