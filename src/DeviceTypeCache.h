//
// Created by stephane bourque on 2022-02-23.
//

#pragma once

#include <set>

#include "framework/AppServiceRegistry.h"
#include "framework/MicroServiceNames.h"
#include "framework/OpenAPIRequests.h"
#include "framework/SubSystemServer.h"

#include "Poco/Timer.h"

namespace OpenWifi {

	class DeviceTypeCache : public SubSystemServer {
	  public:
		inline static auto instance() {
			static auto instance_ = new DeviceTypeCache;
			return instance_;
		}

		inline int Start() final {
			InitializeCache();
			TimerCallback_ = std::make_unique<Poco::TimerCallback<DeviceTypeCache>>(
				*this, &DeviceTypeCache::onTimer);
			Timer_.setStartInterval(60 * 1000);				// first run in 60 seconds
			Timer_.setPeriodicInterval(1 * 60 * 60 * 1000); // 1 hours
			Timer_.start(*TimerCallback_);
			return 0;
		}

		inline void Stop() final { Timer_.stop(); }

		inline void onTimer([[maybe_unused]] Poco::Timer &timer) { UpdateDeviceTypes(); }

		inline bool IsAcceptableDeviceType(const std::string &D) const {
			return (DeviceTypes_.find(D) != DeviceTypes_.end());
		};
		inline bool AreAcceptableDeviceTypes(const Types::StringVec &S,
											 bool WildCardAllowed = true) const {
			for (const auto &i : S) {
				if (WildCardAllowed && i == "*") {
					//   We allow wildcards
				} else if (DeviceTypes_.find(i) == DeviceTypes_.end())
					return false;
			}
			return true;
		}

	  private:
		std::atomic_bool Initialized_ = false;
		Poco::Timer Timer_;
		std::set<std::string> DeviceTypes_;
		std::unique_ptr<Poco::TimerCallback<DeviceTypeCache>> TimerCallback_;

		inline DeviceTypeCache() noexcept
			: SubSystemServer("DeviceTypes", "DEV-TYPES", "devicetypes") {}

		inline void InitializeCache() {
			std::lock_guard G(Mutex_);

			Initialized_ = true;
			std::string DeviceTypes;
			if (AppServiceRegistry().Get("deviceTypes", DeviceTypes)) {
				Poco::JSON::Parser P;
				try {
					auto O = P.parse(DeviceTypes).extract<Poco::JSON::Array::Ptr>();
					for (const auto &i : *O) {
						DeviceTypes_.insert(i.toString());
					}
				} catch (...) {
				}
			}
		}

		inline bool UpdateDeviceTypes() {
			try {
				Types::StringPairVec QueryData;

				QueryData.push_back(std::make_pair("deviceSet", "true"));
				OpenAPIRequestGet Req(uSERVICE_FIRMWARE, "/api/v1/firmwares", QueryData, 10000);

				auto Response = Poco::makeShared<Poco::JSON::Object>();
				auto StatusCode = Req.Do(Response);
				if (StatusCode == Poco::Net::HTTPResponse::HTTP_OK) {
					if (Response->isArray("deviceTypes")) {
						std::lock_guard G(Mutex_);
						DeviceTypes_.clear();
						auto Array = Response->getArray("deviceTypes");
						for (const auto &i : *Array) {
							// std::cout << "Adding deviceType:" << i.toString() << std::endl;
							DeviceTypes_.insert(i.toString());
						}
						SaveCache();
						return true;
					}
				} else {
				}
			} catch (const Poco::Exception &E) {
				Logger().log(E);
			}
			return false;
		}

		inline void SaveCache() {
			std::lock_guard G(Mutex_);

			Poco::JSON::Array Arr;
			for (auto const &i : DeviceTypes_)
				Arr.add(i);

			std::stringstream OS;
			Arr.stringify(OS);

			AppServiceRegistry().Set("deviceTypes", OS.str());
		}
	};

	inline auto DeviceTypeCache() { return DeviceTypeCache::instance(); }

} // namespace OpenWifi