//
// Created by stephane bourque on 2022-04-01.
//

#pragma once

#include "APConfig.h"
#include "JobController.h"
#include "StorageService.h"
#include "UI_Prov_WebSocketNotifications.h"
#include "framework/MicroServiceFuncs.h"
#include "sdks/SDK_gw.h"

namespace OpenWifi {

	[[maybe_unused]] static void GetRejectedLines(const Poco::JSON::Object::Ptr &Response,
												  Types::StringVec &Warnings) {
		try {
			if (Response->has("results")) {
				auto Results = Response->get("results").extract<Poco::JSON::Object::Ptr>();
				auto Status = Results->get("status").extract<Poco::JSON::Object::Ptr>();
				auto Rejected = Status->getArray("rejected");
				std::transform(
					Rejected->begin(), Rejected->end(), std::back_inserter(Warnings),
					[](auto i) -> auto { return i.toString(); });
			}
		} catch (...) {
		}
	}

	[[maybe_unused]] static void ComputeAndPushConfig(const std::string &SerialNumber, const std::string &DeviceType, Poco::Logger &Logger) {
		/*
		Generic Helper to compute a device's config and push it down to the device.
		*/
		poco_information(Logger, fmt::format("Attempting to push venue config for device {}", SerialNumber));
		auto DeviceConfig = std::make_shared<APConfig>(SerialNumber,
														DeviceType, Logger, false);
		auto Configuration = Poco::makeShared<Poco::JSON::Object>();
		try {
			if (DeviceConfig->Get(Configuration)) {
				std::ostringstream OS;
				Configuration->stringify(OS);
				auto Response = Poco::makeShared<Poco::JSON::Object>();
				poco_debug(Logger,
							fmt::format("{}: Pushing configuration.", SerialNumber));
				if (SDK::GW::Device::Configure(nullptr, SerialNumber, Configuration,
												Response)) {
					Logger.debug(
						fmt::format("{}: Configuration pushed.", SerialNumber));
					poco_information(Logger,
										fmt::format("{}: Updated.", SerialNumber));
				} else {
					poco_information(Logger,
										fmt::format("{}: Not updated.", SerialNumber));
				}
			} else {
				poco_debug(Logger,
							fmt::format("{}: Configuration is bad.", SerialNumber));
			}
		} catch (...) {
			poco_debug(Logger,
						fmt::format("{}: Configuration is bad (caused an exception).",
									SerialNumber));
		}
	}

	class VenueDeviceConfigUpdater : public Poco::Runnable {
	  public:
		VenueDeviceConfigUpdater(const std::string &UUID, const std::string &venue, Poco::Logger &L)
			: uuid_(UUID), venue_(venue), Logger_(L) {}

		void run() final {
			ProvObjects::InventoryTag Device;
			started_ = true;
			Utils::SetThreadName("venue-cfg");
			if (StorageService()->InventoryDB().GetRecord("id", uuid_, Device)) {
				SerialNumber = Device.serialNumber;
				// std::cout << "Starting push for " << Device.serialNumber << std::endl;
				Logger().debug(fmt::format("{}: Computing configuration.", Device.serialNumber));
				auto DeviceConfig = std::make_shared<APConfig>(Device.serialNumber,
															   Device.deviceType, Logger(), false);
				auto Configuration = Poco::makeShared<Poco::JSON::Object>();
				try {
					if (DeviceConfig->Get(Configuration)) {
						std::ostringstream OS;
						Configuration->stringify(OS);
						auto Response = Poco::makeShared<Poco::JSON::Object>();
						poco_debug(Logger(),
								   fmt::format("{}: Pushing configuration.", Device.serialNumber));
						if (SDK::GW::Device::Configure(nullptr, Device.serialNumber, Configuration,
													   Response)) {
							Logger().debug(
								fmt::format("{}: Configuration pushed.", Device.serialNumber));
							poco_information(Logger(),
											 fmt::format("{}: Updated.", Device.serialNumber));
							// std::cout << Device.serialNumber << ": Updated" << std::endl;
							updated_++;
						} else {
							poco_information(Logger(),
											 fmt::format("{}: Not updated.", Device.serialNumber));
							// std::cout << Device.serialNumber << ": Failed" << std::endl;
							failed_++;
						}
					} else {
						poco_debug(Logger(),
								   fmt::format("{}: Configuration is bad.", Device.serialNumber));
						bad_config_++;
						// std::cout << Device.serialNumber << ": Bad config" << std::endl;
					}
				} catch (...) {
					poco_debug(Logger(),
							   fmt::format("{}: Configuration is bad (caused an exception).",
										   Device.serialNumber));
					bad_config_++;
				}
			}
			done_ = true;
			// std::cout << "Done push for " << Device.serialNumber << std::endl;
			Utils::SetThreadName("free");
		}

		uint64_t updated_ = 0, failed_ = 0, bad_config_ = 0;
		bool started_ = false, done_ = false;
		std::string SerialNumber;

	  private:
		std::string uuid_;
		std::string venue_;
		Poco::Logger &Logger_;
		inline Poco::Logger &Logger() { return Logger_; }
	};

	class VenueConfigUpdater : public Job {
	  public:
		VenueConfigUpdater(const std::string &JobID, const std::string &name,
						   const std::vector<std::string> &parameters, uint64_t when,
						   const SecurityObjects::UserInfo &UI, Poco::Logger &L)
			: Job(JobID, name, parameters, when, UI, L) {}

		inline virtual void run() {
			std::string VenueUUID_;

			Utils::SetThreadName("venue-update");
			VenueUUID_ = Parameter(0);

			ProvWebSocketNotifications::ConfigUpdateList_t N;

			ProvObjects::Venue Venue;
			uint64_t Updated = 0, Failed = 0, BadConfigs = 0;
			if (StorageService()->VenueDB().GetRecord("id", VenueUUID_, Venue)) {

				N.content.title = fmt::format("Updating {} configurations", Venue.info.name);
				N.content.jobId = JobId();

				Poco::ThreadPool Pool_;
				std::list<VenueDeviceConfigUpdater *> JobList;
                std::vector<std::string> DeviceList;
                StorageService()->InventoryDB().GetDevicesUUIDForVenue(Venue.info.id, DeviceList);
				for (const auto &uuid : DeviceList) {
					auto NewTask = new VenueDeviceConfigUpdater(uuid, Venue.info.name, Logger());
					bool TaskAdded = false;
					while (!TaskAdded) {
						if (Pool_.available()) {
							JobList.push_back(NewTask);
							Pool_.start(*NewTask);
							TaskAdded = true;
							continue;
						}
					}

					for (auto job_it = JobList.begin(); job_it != JobList.end();) {
						VenueDeviceConfigUpdater *current_job = *job_it;
						if (current_job != nullptr && current_job->done_) {
							Updated += current_job->updated_;
							Failed += current_job->failed_;
							BadConfigs += current_job->bad_config_;
							if (current_job->updated_) {
								N.content.success.push_back(current_job->SerialNumber);
							} else if (current_job->failed_) {
								N.content.warning.push_back(current_job->SerialNumber);
							} else {
								N.content.error.push_back(current_job->SerialNumber);
							}
							job_it = JobList.erase(job_it);
							delete current_job;
						} else {
							++job_it;
						}
					}
				}

				poco_debug(Logger(), "Waiting for outstanding update threads to finish.");
				Pool_.joinAll();
				for (auto job_it = JobList.begin(); job_it != JobList.end();) {
					VenueDeviceConfigUpdater *current_job = *job_it;
					if (current_job != nullptr && current_job->done_) {
						Updated += current_job->updated_;
						Failed += current_job->failed_;
						BadConfigs += current_job->bad_config_;
						if (current_job->updated_) {
							N.content.success.push_back(current_job->SerialNumber);
						} else if (current_job->failed_) {
							N.content.warning.push_back(current_job->SerialNumber);
						} else {
							N.content.error.push_back(current_job->SerialNumber);
						}
						job_it = JobList.erase(job_it);
						delete current_job;
					} else {
						++job_it;
					}
				}

				N.content.details = fmt::format(
					"Job {} Completed: {} updated, {} failed to update, {} bad configurations. ",
					JobId(), Updated, Failed, BadConfigs);

			} else {
				N.content.details = fmt::format("Venue {} no longer exists.", VenueUUID_);
				poco_warning(Logger(), N.content.details);
			}

			// std::cout << N.content.details << std::endl;
			ProvWebSocketNotifications::VenueConfigUpdateCompletion(UserInfo().email, N);
			poco_information(
				Logger(),
				fmt::format(
					"Job {} Completed: {} updated, {} failed to update , {} bad configurations.",
					JobId(), Updated, Failed, BadConfigs));
			Utils::SetThreadName("free");
			Complete();
		}
	};

} // namespace OpenWifi