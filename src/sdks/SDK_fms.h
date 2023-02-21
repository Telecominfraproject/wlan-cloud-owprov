//
// Created by stephane bourque on 2022-05-12.
//

#pragma once

#include "RESTObjects/RESTAPI_FMSObjects.h"

namespace OpenWifi::SDK::FMS {

	namespace Firmware {
		bool GetLatest(const std::string &device_type, bool RCOnly, FMSObjects::Firmware &FirmWare);
		bool GetDeviceTypeFirmwares(const std::string &device_type,
									std::vector<FMSObjects::Firmware> &FirmWares);
		bool GetFirmware(const std::string &device_type, const std::string &revision,
						 FMSObjects::Firmware &FirmWare);
	} // namespace Firmware

}; // namespace OpenWifi::SDK::FMS
