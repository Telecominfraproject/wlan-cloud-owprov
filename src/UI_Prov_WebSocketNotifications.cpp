//
// Created by stephane bourque on 2022-10-29.
//

#include "UI_Prov_WebSocketNotifications.h"

namespace OpenWifi::ProvWebSocketNotifications {

	void ConfigUpdateList::to_json(Poco::JSON::Object &Obj) const {
		RESTAPI_utils::field_to_json(Obj, "title", title);
		RESTAPI_utils::field_to_json(Obj, "jobId", jobId);
		RESTAPI_utils::field_to_json(Obj, "success", success);
		RESTAPI_utils::field_to_json(Obj, "error", error);
		RESTAPI_utils::field_to_json(Obj, "warning", warning);
		RESTAPI_utils::field_to_json(Obj, "timeStamp", timeStamp);
		RESTAPI_utils::field_to_json(Obj, "details", details);
	}

	bool ConfigUpdateList::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			RESTAPI_utils::field_from_json(Obj, "title", title);
			RESTAPI_utils::field_from_json(Obj, "jobId", jobId);
			RESTAPI_utils::field_from_json(Obj, "success", success);
			RESTAPI_utils::field_from_json(Obj, "error", error);
			RESTAPI_utils::field_from_json(Obj, "warning", warning);
			RESTAPI_utils::field_from_json(Obj, "timeStamp", timeStamp);
			RESTAPI_utils::field_from_json(Obj, "details", details);
			return true;
		} catch (...) {
		}
		return false;
	}

	void RebootList::to_json(Poco::JSON::Object &Obj) const {
		RESTAPI_utils::field_to_json(Obj, "title", title);
		RESTAPI_utils::field_to_json(Obj, "jobId", jobId);
		RESTAPI_utils::field_to_json(Obj, "success", success);
		RESTAPI_utils::field_to_json(Obj, "warning", warning);
		RESTAPI_utils::field_to_json(Obj, "timeStamp", timeStamp);
		RESTAPI_utils::field_to_json(Obj, "details", details);
	}

	bool RebootList::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			RESTAPI_utils::field_from_json(Obj, "title", title);
			RESTAPI_utils::field_from_json(Obj, "jobId", jobId);
			RESTAPI_utils::field_from_json(Obj, "success", success);
			RESTAPI_utils::field_from_json(Obj, "warning", warning);
			RESTAPI_utils::field_from_json(Obj, "timeStamp", timeStamp);
			RESTAPI_utils::field_from_json(Obj, "details", details);
			return true;
		} catch (...) {
		}
		return false;
	}

	void FWUpgradeList::to_json(Poco::JSON::Object &Obj) const {
		RESTAPI_utils::field_to_json(Obj, "title", title);
		RESTAPI_utils::field_to_json(Obj, "jobId", jobId);
		RESTAPI_utils::field_to_json(Obj, "success", success);
		RESTAPI_utils::field_to_json(Obj, "notConnected", not_connected);
		RESTAPI_utils::field_to_json(Obj, "noFirmware", no_firmware);
        RESTAPI_utils::field_to_json(Obj, "pending", pending);
		RESTAPI_utils::field_to_json(Obj, "skipped", skipped);
		RESTAPI_utils::field_to_json(Obj, "timeStamp", timeStamp);
		RESTAPI_utils::field_to_json(Obj, "details", details);
	}

	bool FWUpgradeList::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			RESTAPI_utils::field_from_json(Obj, "title", title);
			RESTAPI_utils::field_from_json(Obj, "jobId", jobId);
			RESTAPI_utils::field_from_json(Obj, "success", success);
			RESTAPI_utils::field_from_json(Obj, "notConnected", not_connected);
            RESTAPI_utils::field_from_json(Obj, "pending", pending);
			RESTAPI_utils::field_from_json(Obj, "noFirmware", no_firmware);
			RESTAPI_utils::field_from_json(Obj, "skipped", skipped);
			RESTAPI_utils::field_from_json(Obj, "timeStamp", timeStamp);
			RESTAPI_utils::field_from_json(Obj, "details", details);
			return true;
		} catch (...) {
		}
		return false;
	}

	void Register() {
		static const UI_WebSocketClientServer::NotificationTypeIdVec Notifications = {
			{1000, "venue_fw_upgrade"}, {2000, "venue_config_update"}, {3000, "venue_rebooter"}};
		UI_WebSocketClientServer()->RegisterNotifications(Notifications);
	}

	void VenueFWUpgradeCompletion(VenueFWUpgradeList_t &N) {
		N.type_id = 1000;
		UI_WebSocketClientServer()->SendNotification(N);
	}

	void VenueFWUpgradeCompletion(const std::string &User, VenueFWUpgradeList_t &N) {
		N.type_id = 1000;
		UI_WebSocketClientServer()->SendUserNotification(User, N);
	}

	void VenueConfigUpdateCompletion(ConfigUpdateList_t &N) {
		N.type_id = 2000;
		UI_WebSocketClientServer()->SendNotification(N);
	}

	void VenueConfigUpdateCompletion(const std::string &User, ConfigUpdateList_t &N) {
		N.type_id = 2000;
		UI_WebSocketClientServer()->SendUserNotification(User, N);
	}

	void VenueRebootCompletion(VenueRebootList_t &N) {
		N.type_id = 3000;
		UI_WebSocketClientServer()->SendNotification(N);
	}

	void VenueRebootCompletion(const std::string &User, VenueRebootList_t &N) {
		N.type_id = 3000;
		UI_WebSocketClientServer()->SendUserNotification(User, N);
	}

} // namespace OpenWifi::ProvWebSocketNotifications
