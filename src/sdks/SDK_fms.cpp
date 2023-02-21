//
// Created by stephane bourque on 2022-05-12.
//

#include "SDK_fms.h"

#include "RESTObjects/RESTAPI_FMSObjects.h"

#include "framework/MicroServiceNames.h"
#include "framework/OpenAPIRequests.h"

namespace OpenWifi::SDK::FMS {

	namespace Firmware {

		bool GetLatest(const std::string &device_type, bool RCOnly,
					   FMSObjects::Firmware &FirmWare) {
			static const std::string EndPoint{"/api/v1/firmwares"};

			OpenWifi::OpenAPIRequestGet API(uSERVICE_FIRMWARE, EndPoint,
											{{"latestOnly", "true"},
											 {"deviceType", device_type},
											 {"rcOnly", RCOnly ? "true" : "false"}},
											50000);

			auto CallResponse = Poco::makeShared<Poco::JSON::Object>();
			auto StatusCode = API.Do(CallResponse);
			if (StatusCode == Poco::Net::HTTPResponse::HTTP_OK) {
				return FirmWare.from_json(CallResponse);
			}
			return false;
		}

		bool GetDeviceTypeFirmwares(const std::string &device_type,
									std::vector<FMSObjects::Firmware> &FirmWares) {
			static const std::string EndPoint{"/api/v1/firmwares"};

			OpenWifi::OpenAPIRequestGet API(uSERVICE_FIRMWARE, EndPoint,
											{{"deviceType", device_type}}, 50000);

			auto CallResponse = Poco::makeShared<Poco::JSON::Object>();
			auto StatusCode = API.Do(CallResponse);
			if (StatusCode == Poco::Net::HTTPResponse::HTTP_OK) {
				Poco::JSON::Array::Ptr FirmwareArr = CallResponse->getArray("firmwares");
				for (uint64_t i = 0; i < FirmwareArr->size(); i++) {
					FMSObjects::Firmware F;
					F.from_json(FirmwareArr->getObject(i));
					FirmWares.emplace_back(F);
				}
				return true;
			}
			return false;
		}

		bool GetFirmware(const std::string &device_type, const std::string &revision,
						 FMSObjects::Firmware &Firmware) {
			std::vector<FMSObjects::Firmware> Firmwares;
			if (GetDeviceTypeFirmwares(device_type, Firmwares)) {
				for (const auto &firmware : Firmwares) {
					if (firmware.revision == revision) {
						Firmware = firmware;
						return true;
					}
				}
			}
			return false;
		}

	} // namespace Firmware

}; // namespace OpenWifi::SDK::FMS
