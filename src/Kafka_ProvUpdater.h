//
// Created by stephane bourque on 2022-04-01.
//

#pragma once

#include "RESTObjects/RESTAPI_ProvObjects.h"
#include "framework/KafkaManager.h"
#include "framework/KafkaTopics.h"

namespace OpenWifi {

	enum ProvisioningOperation { creation = 0, modification, removal };

	template <typename ObjectType>
	inline bool UpdateKafkaProvisioningObject(ProvisioningOperation op, const ObjectType &obj) {
		static std::vector<std::string> Ops{"creation", "modification", "removal"};

		std::string OT{"object"};
		if constexpr (std::is_same_v<ObjectType, ProvObjects::Venue>) {
			OT = "Venue";
		}
		if constexpr (std::is_same_v<ObjectType, ProvObjects::Entity>) {
			OT = "Entity";
		}
		if constexpr (std::is_same_v<ObjectType, ProvObjects::InventoryTag>) {
			OT = "InventoryTag";
		}
		if constexpr (std::is_same_v<ObjectType, ProvObjects::Contact>) {
			OT = "Contact";
		}
		if constexpr (std::is_same_v<ObjectType, ProvObjects::Location>) {
			OT = "Location";
		}
		if constexpr (std::is_same_v<ObjectType, ProvObjects::DeviceConfiguration>) {
			OT = "DeviceConfiguration";
		}

		Poco::JSON::Object Payload;
		obj.to_json(Payload);
		Payload.set("ObjectType", OT);
		KafkaManager()->PostMessage(KafkaTopics::PROVISIONING_CHANGE, Ops[op], Payload);

		return true;
	}
} // namespace OpenWifi