//
// Created by stephane bourque on 2022-05-04.
//

#include "framework/MicroService.h"
#include "framework/WebSocketClientNotifications.h"

#include "StorageService.h"
#include "APConfig.h"
#include "sdks/SDK_gw.h"
#include "JobController.h"

namespace OpenWifi {

    class VenueDeviceRebooter : public Poco::Runnable {
    public:
        VenueDeviceRebooter(const std::string &UUID, const std::string &venue, Poco::Logger &L) :
                uuid_(UUID),
                venue_(venue),
                Logger_(L) {
        }

        void run() final {
            ProvObjects::InventoryTag   Device;
            started_=true;
            if(StorageService()->InventoryDB().GetRecord("id",uuid_,Device)) {
                SerialNumber = Device.serialNumber;
                if (SDK::GW::Device::Reboot(Device.serialNumber, 0)) {
                    Logger().debug(fmt::format("{}: Rebooted.",Device.serialNumber));
                    rebooted_++;
                } else {
                    Logger().information(fmt::format("{}: Not rebooted.", Device.serialNumber));
                    failed_++;
                }
            }
            done_ = true;
            // std::cout << "Done push for " << Device.serialNumber << std::endl;
        }

        uint64_t        rebooted_=0, failed_=0;
        bool            started_ = false,
                        done_ = false;
        std::string     SerialNumber;

    private:
        std::string     uuid_;
        std::string     venue_;
        Poco::Logger    &Logger_;
        inline Poco::Logger & Logger() { return Logger_; }
    };

    class VenueRebooter: public Job {
    public:
        VenueRebooter(const std::string &JobID, const std::string &name, const std::vector<std::string> & parameters, uint64_t when, const SecurityObjects::UserInfo &UI, Poco::Logger &L) :
                Job(JobID, name, parameters, when, UI, L) {

        }

        inline virtual void run() final {

            Utils::SetThreadName("venue-reboot");

            WebSocketClientNotificationVenueRebootList_t        N;
            auto VenueUUID_ = Parameter(0);

            ProvObjects::Venue  Venue;
            uint64_t rebooted_ = 0, failed_ = 0;
            if(StorageService()->VenueDB().GetRecord("id",VenueUUID_,Venue)) {
                const std::size_t MaxThreads=16;
                struct tState {
                    Poco::Thread                thr_;
                    VenueDeviceRebooter    *task= nullptr;
                };

                N.content.title = fmt::format("Rebooting {} devices.", Venue.info.name);
                N.content.jobId = JobId();

                std::array<tState,MaxThreads> Tasks;

                for(const auto &uuid:Venue.devices) {
                    auto NewTask = new VenueDeviceRebooter(uuid, Venue.info.name, Logger());
                    // std::cout << "Scheduling config push for " << uuid << std::endl;
                    bool found_slot = false;
                    while (!found_slot) {
                        for (auto &cur_task: Tasks) {
                            if (cur_task.task == nullptr) {
                                cur_task.task = NewTask;
                                cur_task.thr_.start(*NewTask);
                                found_slot = true;
                                break;
                            }
                        }

                        //  Let's look for a slot...
                        if (!found_slot) {
                            for (auto &cur_task: Tasks) {
                                if (cur_task.task != nullptr && cur_task.task->started_) {
                                    if (cur_task.thr_.isRunning())
                                        continue;
                                    if (!cur_task.thr_.isRunning() && cur_task.task->done_) {
                                        cur_task.thr_.join();
                                        rebooted_ += cur_task.task->rebooted_;
                                        failed_ += cur_task.task->failed_;
                                        cur_task.task->started_ = cur_task.task->done_ = false;
                                        delete cur_task.task;
                                        cur_task.task = nullptr;
                                    }
                                }
                            }
                        }
                    }
                }
                Logger().debug("Waiting for outstanding update threads to finish.");
                bool stillTasksRunning=true;
                while(stillTasksRunning) {
                    stillTasksRunning = false;
                    for(auto &cur_task:Tasks) {
                        if(cur_task.task!= nullptr && cur_task.task->started_) {
                            if(cur_task.thr_.isRunning()) {
                                stillTasksRunning = true;
                                continue;
                            }
                            if(!cur_task.thr_.isRunning() && cur_task.task->done_) {
                                cur_task.thr_.join();
                                if(cur_task.task->rebooted_) {
                                    rebooted_++;
                                    N.content.success.push_back(cur_task.task->SerialNumber);
                                } else if(cur_task.task->failed_) {
                                    failed_++;
                                    N.content.warning.push_back(cur_task.task->SerialNumber);
                                }
                                cur_task.task->started_ = cur_task.task->done_ = false;
                                delete cur_task.task;
                                cur_task.task = nullptr;
                            }
                        }
                    }
                }

                N.content.details = fmt::format("Job {} Completed: {} rebooted, {} failed to reboot.",
                                                JobId(), rebooted_ ,failed_);

            } else {
                N.content.details = fmt::format("Venue {} no longer exists.",VenueUUID_);
                Logger().warning(N.content.details);
            }

            WebSocketClientNotificationVenueRebootCompletionToUser(UserInfo().email,N);
            Logger().information(fmt::format("Job {} Completed: {} rebooted, {} failed to reboot.",
                                             JobId(), rebooted_ ,failed_));
            Utils::SetThreadName("free");
            Complete();
        }
    };

}