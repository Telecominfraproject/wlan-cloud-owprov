//
// Created by stephane bourque on 2022-11-03.
//

#include <algorithm>

#include "RESTAPI_overrides_handler.h"
#include "framework/utils.h"

namespace OpenWifi {
	void RESTAPI_overrides_handler::DoGet() {
		std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER, "");

		if (!Utils::NormalizeMac(SerialNumber)) {
			return BadRequest(RESTAPI::Errors::InvalidSerialNumber);
		}

		ProvObjects::ConfigurationOverrideList ExistingObject;
		if (!DB_.GetRecord("serialNumber", SerialNumber, ExistingObject)) {
			return NotFound();
		}
		Poco::JSON::Object Answer;
		ExistingObject.to_json(Answer);

		return ReturnObject(Answer);
	}

	void RESTAPI_overrides_handler::DoDelete() {
		std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER, "");

		if (!Utils::NormalizeMac(SerialNumber)) {
			return BadRequest(RESTAPI::Errors::InvalidSerialNumber);
		}

		auto Source = GetParameter("source", "");
		if (Source.empty()) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		ProvObjects::ConfigurationOverrideList ExistingObject;
		if (!DB_.GetRecord("serialNumber", SerialNumber, ExistingObject)) {
			return NotFound();
		}

		ExistingObject.overrides.erase(
			std::remove_if(ExistingObject.overrides.begin(), ExistingObject.overrides.end(),
						   [Source](const ProvObjects::ConfigurationOverride &O) -> bool {
							   return O.source == Source;
						   }),
			ExistingObject.overrides.end());

		if (DB_.UpdateRecord("serialNumber", SerialNumber, ExistingObject)) {
			return OK();
		}
		return BadRequest(RESTAPI::Errors::NoRecordsDeleted);
	}

	void RESTAPI_overrides_handler::DoPut() {
		std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER, "");

		if (!Utils::NormalizeMac(SerialNumber)) {
			return BadRequest(RESTAPI::Errors::InvalidSerialNumber);
		}

		auto Source = GetParameter("source", "");
		if (Source.empty()) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		ProvObjects::ConfigurationOverrideList NewObject;
		if (!NewObject.from_json(ParsedBody_)) {
			return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
		}

		ProvObjects::ConfigurationOverrideList ExistingObject;
		if (!DB_.GetRecord("serialNumber", SerialNumber, ExistingObject)) {
			ExistingObject.serialNumber = SerialNumber;
			DB_.CreateRecord(ExistingObject);
		} else {
			// remove all the old records with that source.
			ExistingObject.overrides.erase(
				std::remove_if(ExistingObject.overrides.begin(), ExistingObject.overrides.end(),
							   [Source](const ProvObjects::ConfigurationOverride &O) -> bool {
								   return O.source == Source;
							   }),
				ExistingObject.overrides.end());
		}

		for (auto &override : NewObject.overrides) {
			if (override.parameterName.empty()) {
				continue;
			}
			if (override.parameterType != "string" && override.parameterType != "boolean" &&
				override.parameterType != "integer") {
				return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
			}
			override.source = Source;
			override.modified = Utils::Now();
			ExistingObject.overrides.emplace_back(override);
		}

		if (DB_.UpdateRecord("serialNumber", SerialNumber, ExistingObject)) {
			Poco::JSON::Object Answer;
			ExistingObject.to_json(Answer);
			return ReturnObject(Answer);
		}

		return BadRequest(RESTAPI::Errors::RecordNotUpdated);
	}

} // namespace OpenWifi