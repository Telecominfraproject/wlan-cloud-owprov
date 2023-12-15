//
// Created by stephane bourque on 2022-10-29.
//

#pragma once

#include "framework/UI_WebSocketClientNotifications.h"
#include "framework/UI_WebSocketClientServer.h"

namespace OpenWifi::ProvWebSocketNotifications {
	struct ConfigUpdateList {
		std::string title, details, jobId;
		std::vector<std::string> success, error, warning;
		uint64_t timeStamp = OpenWifi::Utils::Now();

		void to_json(Poco::JSON::Object &Obj) const;
		bool from_json(const Poco::JSON::Object::Ptr &Obj);
	};

	typedef WebSocketNotification<ConfigUpdateList> ConfigUpdateList_t;

	struct RebootList {
		std::string title, details, jobId;
		std::vector<std::string> success, warning;
		uint64_t timeStamp = OpenWifi::Utils::Now();

		void to_json(Poco::JSON::Object &Obj) const;
		bool from_json(const Poco::JSON::Object::Ptr &Obj);
	};

	typedef WebSocketNotification<RebootList> VenueRebootList_t;

	struct FWUpgradeList {
		std::string title, details, jobId;
		std::vector<std::string> success, skipped, no_firmware, not_connected, pending;
		uint64_t timeStamp = OpenWifi::Utils::Now();

		void to_json(Poco::JSON::Object &Obj) const;

		bool from_json(const Poco::JSON::Object::Ptr &Obj);
	};

	typedef WebSocketNotification<FWUpgradeList> VenueFWUpgradeList_t;

	void Register();

	void VenueFWUpgradeCompletion(const std::string &User, VenueFWUpgradeList_t &N);
	void VenueFWUpgradeCompletion(VenueFWUpgradeList_t &N);

	void VenueConfigUpdateCompletion(const std::string &User, ConfigUpdateList_t &N);
	void VenueConfigUpdateCompletion(ConfigUpdateList_t &N);

	void VenueRebootCompletion(const std::string &User, VenueRebootList_t &N);
	void VenueRebootCompletion(VenueRebootList_t &N);
} // namespace OpenWifi::ProvWebSocketNotifications
// namespace OpenWifi
