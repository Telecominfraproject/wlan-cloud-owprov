//
// Created by stephane bourque on 2021-10-28.
//

#include "JobController.h"
#include "fmt/format.h"
#include "framework/utils.h"

namespace OpenWifi {

	void RegisterJobTypes();

	int JobController::Start() {
		poco_information(Logger(), "Starting...");
		RegisterJobTypes();
		if (!Running_)
			Thr_.start(*this);

		return 0;
	}

	void JobController::Stop() {
		if (Running_) {
			poco_information(Logger(), "Stopping...");
			Running_ = false;
			Thr_.join();
			poco_information(Logger(), "Stopped...");
		}
	}

	void JobController::run() {
		Running_ = true;
		Utils::SetThreadName("job-controller");
		while (Running_) {
			Poco::Thread::trySleep(2000);

			std::lock_guard G(Mutex_);

			for (auto &current_job : jobs_) {
				if (current_job != nullptr) {
					if (current_job->Started() == 0 && Pool_.used() < Pool_.available()) {
						poco_information(current_job->Logger(),
										 fmt::format("Starting {}: {}", current_job->JobId(),
													 current_job->Name()));
						current_job->Start();
						Pool_.start(*current_job);
					}
				}
			}

			for (auto it = jobs_.begin(); it != jobs_.end();) {
				auto current_job = *it;
				if (current_job != nullptr && current_job->Completed() != 0) {
					poco_information(
						current_job->Logger(),
						fmt::format("Completed {}: {}", current_job->JobId(), current_job->Name()));
					it = jobs_.erase(it);
					delete current_job;
				} else {
					++it;
				}
			}
		}
	}
} // namespace OpenWifi