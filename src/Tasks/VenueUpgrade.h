//
// Created by stephane bourque on 2022-05-12.
//

#pragma once

#include "APConfig.h"
#include "JobController.h"
#include "StorageService.h"
#include "UI_Prov_WebSocketNotifications.h"
#include "framework/MicroServiceFuncs.h"
#include "sdks/SDK_fms.h"
#include "sdks/SDK_gw.h"

namespace OpenWifi {
	class VenueDeviceUpgrade : public Poco::Runnable {
	  public:
		VenueDeviceUpgrade(const std::string &UUID, const std::string &venue,
						   const std::string &revision, const ProvObjects::DeviceRules &Rules,
						   Poco::Logger &L)
			: uuid_(UUID), venue_(venue), revision_(revision), rules_(Rules), Logger_(L) {}

		void run() final {
			ProvObjects::InventoryTag Device;
			started_ = true;
			if (StorageService()->InventoryDB().GetRecord("id", uuid_, Device)) {
				SerialNumber = Device.serialNumber;

				Storage::ApplyRules(rules_, Device.deviceRules);
				if (Device.deviceRules.firmwareUpgrade == "no") {
					poco_debug(Logger(), fmt::format("Skipped Upgrade: {} : Venue rules prevent upgrading", Device.serialNumber));
					skipped_++;
					done_ = true;
					return;
				}

				FMSObjects::Firmware F;
				if (SDK::FMS::Firmware::GetFirmware(Device.deviceType, revision_, F)) {
                    std::string Status;
					if (SDK::GW::Device::Upgrade(nullptr, Device.serialNumber, 0, F.uri, Status)) {
                        if(Status=="pending") {
                            pending_++;
                            poco_debug(Logger(), fmt::format("Upgrade Pending: {} : {}", Device.serialNumber, Status));
                        } else {
                            upgraded_++;
                            poco_debug(Logger(), fmt::format("Upgrade Success: {} : {}", Device.serialNumber, Status));
                        }
					} else {
						poco_information(Logger(), fmt::format("{}: Not Upgraded to {}.",
															   Device.serialNumber, revision_));
						not_connected_++;
					}
				} else {
					poco_information(Logger(),
									 fmt::format("{}: Not Upgraded. No firmware available.",
												 Device.serialNumber));
					no_firmware_++;
				}
			}
			done_ = true;
		}

		std::uint64_t upgraded_ = 0, not_connected_ = 0, skipped_ = 0, no_firmware_ = 0, pending_ = 0;
		bool started_ = false, done_ = false;
		std::string SerialNumber;

	  private:
		std::string uuid_;
		std::string venue_;
		std::string revision_;
		ProvObjects::DeviceRules rules_;
		Poco::Logger &Logger_;
		inline Poco::Logger &Logger() { return Logger_; }
	};

	class VenueUpgrade : public Job {
	  public:
		VenueUpgrade(const std::string &JobID, const std::string &name,
					 const std::vector<std::string> &parameters, uint64_t when,
					 const SecurityObjects::UserInfo &UI, Poco::Logger &L)
			: Job(JobID, name, parameters, when, UI, L) {}

		inline virtual void run() final {

			Utils::SetThreadName("venue-upgr");
			auto VenueUUID_ = Parameter(0);
			auto Revision_ = Parameter(1);

			ProvWebSocketNotifications::VenueFWUpgradeList_t N;

			ProvObjects::Venue Venue;
			uint64_t upgraded_ = 0, not_connected_ = 0, skipped_ = 0, no_firmware_ = 0, pending_=0;
			if (StorageService()->VenueDB().GetRecord("id", VenueUUID_, Venue)) {

				N.content.title = fmt::format("Upgrading {} devices.", Venue.info.name);
				N.content.jobId = JobId();

				Poco::ThreadPool Pool_;
				std::list<VenueDeviceUpgrade *> JobList;
				ProvObjects::DeviceRules Rules;

				StorageService()->VenueDB().EvaluateDeviceRules(Venue.info.id, Rules);
                std::vector<std::string> DeviceList;
                StorageService()->InventoryDB().GetDevicesUUIDForVenue(Venue.info.id, DeviceList);

				for (const auto &uuid : DeviceList) {
					auto NewTask =
						new VenueDeviceUpgrade(uuid, Venue.info.name, Revision_, Rules, Logger());
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
						VenueDeviceUpgrade *current_job = *job_it;
						if (current_job != nullptr && current_job->done_) {
							if (current_job->upgraded_)
								N.content.success.push_back(current_job->SerialNumber);
							else if (current_job->skipped_)
								N.content.skipped.push_back(current_job->SerialNumber);
							else if (current_job->not_connected_)
								N.content.not_connected.push_back(current_job->SerialNumber);
							else if (current_job->no_firmware_)
								N.content.no_firmware.push_back(current_job->SerialNumber);
                            else if (current_job->pending_)
                                N.content.pending.push_back(current_job->SerialNumber);
							upgraded_ += current_job->upgraded_;
							skipped_ += current_job->skipped_;
							no_firmware_ += current_job->no_firmware_;
							not_connected_ += current_job->not_connected_;
                            pending_ += current_job->pending_;
							job_it = JobList.erase(job_it);
							delete current_job;
						} else {
							++job_it;
						}
					}
				}

				Logger().debug("Waiting for outstanding upgrade threads to finish.");
				Pool_.joinAll();
				for (auto job_it = JobList.begin(); job_it != JobList.end();) {
					VenueDeviceUpgrade *current_job = *job_it;
					if (current_job != nullptr && current_job->done_) {
						if (current_job->upgraded_)
							N.content.success.push_back(current_job->SerialNumber);
						else if (current_job->skipped_)
							N.content.skipped.push_back(current_job->SerialNumber);
						else if (current_job->not_connected_)
							N.content.not_connected.push_back(current_job->SerialNumber);
						else if (current_job->no_firmware_)
							N.content.no_firmware.push_back(current_job->SerialNumber);
                        else if (current_job->pending_)
                            N.content.pending.push_back(current_job->SerialNumber);
						upgraded_ += current_job->upgraded_;
						skipped_ += current_job->skipped_;
						no_firmware_ += current_job->no_firmware_;
						not_connected_ += current_job->not_connected_;
                        pending_ += current_job->pending_;
						job_it = JobList.erase(job_it);
						delete current_job;
					} else {
						++job_it;
					}
				}

				N.content.details = fmt::format(
					"Job {} Completed: {} upgraded, {} not connected, {} skipped, {} no firmware, {} pending.",
					JobId(), upgraded_, not_connected_, skipped_, no_firmware_, pending_);
			} else {
				N.content.details = fmt::format("Venue {} no longer exists.", VenueUUID_);
				Logger().warning(N.content.details);
			}

			// std::cout << N.content.details << std::endl;
			ProvWebSocketNotifications::VenueFWUpgradeCompletion(UserInfo().email, N);
			poco_information(Logger(), N.content.details);
			Utils::SetThreadName("free");
			Complete();
		}
	};
} // namespace OpenWifi