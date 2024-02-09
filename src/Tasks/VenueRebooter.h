//
// Created by stephane bourque on 2022-05-04.
//

#include "APConfig.h"
#include "JobController.h"
#include "StorageService.h"
#include "UI_Prov_WebSocketNotifications.h"
#include "framework/MicroServiceFuncs.h"
#include "sdks/SDK_gw.h"

namespace OpenWifi {

	class VenueDeviceRebooter : public Poco::Runnable {
	  public:
		VenueDeviceRebooter(const std::string &UUID, const std::string &venue, Poco::Logger &L)
			: uuid_(UUID), venue_(venue), Logger_(L) {}

		void run() final {
			ProvObjects::InventoryTag Device;
			started_ = true;
			if (StorageService()->InventoryDB().GetRecord("id", uuid_, Device)) {
				SerialNumber = Device.serialNumber;
				if (SDK::GW::Device::Reboot(Device.serialNumber, 0)) {
					Logger().debug(fmt::format("{}: Rebooted.", Device.serialNumber));
					rebooted_++;
				} else {
					poco_information(Logger(),
									 fmt::format("{}: Not rebooted.", Device.serialNumber));
					failed_++;
				}
			}
			done_ = true;
			// std::cout << "Done push for " << Device.serialNumber << std::endl;
		}

		uint64_t rebooted_ = 0, failed_ = 0;
		bool started_ = false, done_ = false;
		std::string SerialNumber;

	  private:
		std::string uuid_;
		std::string venue_;
		Poco::Logger &Logger_;
		inline Poco::Logger &Logger() { return Logger_; }
	};

	class VenueRebooter : public Job {
	  public:
		VenueRebooter(const std::string &JobID, const std::string &name,
					  const std::vector<std::string> &parameters, uint64_t when,
					  const SecurityObjects::UserInfo &UI, Poco::Logger &L)
			: Job(JobID, name, parameters, when, UI, L) {}

		inline virtual void run() final {

			Utils::SetThreadName("venue-reboot");

			ProvWebSocketNotifications::VenueRebootList_t N;
			auto VenueUUID_ = Parameter(0);

			ProvObjects::Venue Venue;
			uint64_t rebooted_ = 0, failed_ = 0;
			if (StorageService()->VenueDB().GetRecord("id", VenueUUID_, Venue)) {

				N.content.title = fmt::format("Rebooting {} devices.", Venue.info.name);
				N.content.jobId = JobId();

				Poco::ThreadPool Pool_;
				std::list<VenueDeviceRebooter *> JobList;
                std::vector<std::string> DeviceList;
                StorageService()->InventoryDB().GetDevicesUUIDForVenue(Venue.info.id, DeviceList);

				for (const auto &uuid : DeviceList) {
					auto NewTask = new VenueDeviceRebooter(uuid, Venue.info.name, Logger());
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
						VenueDeviceRebooter *current_job = *job_it;
						if (current_job != nullptr && current_job->done_) {
							if (current_job->rebooted_)
								N.content.success.push_back(current_job->SerialNumber);
							else
								N.content.warning.push_back(current_job->SerialNumber);
							rebooted_ += current_job->rebooted_;
							failed_ += current_job->failed_;
							job_it = JobList.erase(job_it);
							delete current_job;
						} else {
							++job_it;
						}
					}
				}

				Logger().debug("Waiting for outstanding update threads to finish.");
				Pool_.joinAll();
				for (auto job_it = JobList.begin(); job_it != JobList.end();) {
					VenueDeviceRebooter *current_job = *job_it;
					if (current_job != nullptr && current_job->done_) {
						if (current_job->rebooted_)
							N.content.success.push_back(current_job->SerialNumber);
						else
							N.content.warning.push_back(current_job->SerialNumber);
						rebooted_ += current_job->rebooted_;
						failed_ += current_job->failed_;
						job_it = JobList.erase(job_it);
						delete current_job;
					} else {
						++job_it;
					}
				}
				N.content.details =
					fmt::format("Job {} Completed: {} rebooted, {} failed to reboot.", JobId(),
								rebooted_, failed_);

			} else {
				N.content.details = fmt::format("Venue {} no longer exists.", VenueUUID_);
				Logger().warning(N.content.details);
			}

			// std::cout << N.content.details << std::endl;
			ProvWebSocketNotifications::VenueRebootCompletion(UserInfo().email, N);
			poco_information(Logger(),
							 fmt::format("Job {} Completed: {} rebooted, {} failed to reboot.",
										 JobId(), rebooted_, failed_));
			Utils::SetThreadName("free");
			Complete();
		}
	};

} // namespace OpenWifi