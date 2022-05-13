//
// Created by stephane bourque on 2022-05-12.
//

#pragma once

#include "framework/MicroService.h"
#include "framework/WebSocketClientNotifications.h"

#include "StorageService.h"
#include "APConfig.h"
#include "sdks/SDK_gw.h"
#include "sdks/SDK_fms.h"

namespace OpenWifi {
    class VenueDeviceUpgrade : public Poco::Runnable {
    public:
        VenueDeviceUpgrade(const std::string &UUID, const std::string &venue, [[maybe_unused]] const std::string &fw_rules, Poco::Logger &L) :
                uuid_(UUID),
                venue_(venue),
                Logger_(L) {
        }

        void run() final {
            ProvObjects::InventoryTag   Device;
            started_=true;
            if(StorageService()->InventoryDB().GetRecord("id",uuid_,Device)) {
                SerialNumber = Device.serialNumber;
                // get the latest firmware for device type
                if (SDK::GW::Device::Reboot(Device.serialNumber, 0)) {
                    Logger().debug(fmt::format("{}: Upgraded.",Device.serialNumber));
                    upgraded_++;
                } else {
                    Logger().information(fmt::format("{}: Not Upgraded.", Device.serialNumber));
                    failed_++;
                }
            }
            done_ = true;
            // std::cout << "Done push for " << Device.serialNumber << std::endl;
        }

        uint64_t        upgraded_=0, failed_=0;
        bool            started_ = false,
                        done_ = false;
        std::string     SerialNumber;

    private:
        std::string     uuid_;
        std::string     venue_;
        Poco::Logger    &Logger_;
        inline Poco::Logger & Logger() { return Logger_; }
    };

    class VenueUpgrade: public Poco::Runnable {
    public:
        explicit VenueUpgrade(const std::string & VenueUUID, const SecurityObjects::UserInfo &UI, uint64_t When, Poco::Logger &L) :
                VenueUUID_(VenueUUID),
                UI_(UI),
                When_(When),
                Logger_(L)
        {

        }

        inline std::string Start() {
            JobId_ = MicroService::CreateUUID();
            Worker_.start(*this);
            return JobId_;
        }

    private:
        std::string                 VenueUUID_;
        SecurityObjects::UserInfo   UI_;
        uint64_t                    When_;
        Poco::Logger                &Logger_;
        Poco::Thread                Worker_;
        std::string                 JobId_;
        Poco::ThreadPool            Pool_{2,16,300};

        inline Poco::Logger & Logger() { return Logger_; }

        inline void run() final {

            if(When_ && When_>OpenWifi::Now())
                Poco::Thread::trySleep( (long) (When_ - OpenWifi::Now()) * 1000 );

            WebSocketClientNotificationVenueRebootList_t        N;

            Logger().information(fmt::format("Job {} Starting.", JobId_));

            ProvObjects::Venue  Venue;
            uint64_t upgraded_ = 0, failed_ = 0;
            if(StorageService()->VenueDB().GetRecord("id",VenueUUID_,Venue)) {
                const std::size_t MaxThreads=16;
                struct tState {
                    Poco::Thread                thr_;
                    VenueDeviceUpgrade    *task= nullptr;
                };

                N.content.title = fmt::format("Upgrading {} devices.", Venue.info.name);
                N.content.jobId = JobId_;

                std::array<tState,MaxThreads> Tasks;

                for(const auto &uuid:Venue.devices) {
                    std::string fw_rules;
                    auto NewTask = new VenueDeviceUpgrade(uuid, Venue.info.name, fw_rules,Logger());
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
                                        upgraded_ += cur_task.task->upgraded_;
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
                                if(cur_task.task->upgraded_) {
                                    upgraded_++;
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

                N.content.details = fmt::format("Job {} Completed: {} upgraded, {} failed to upgrade.",
                                                JobId_, upgraded_ ,failed_);

            } else {
                N.content.details = fmt::format("Venue {} no longer exists.",VenueUUID_);
                Logger().warning(N.content.details);
            }

            WebSocketClientNotificationVenueRebootCompletionToUser(UI_.email,N);
            Logger().information(fmt::format("Job {} Completed: {} upgraded, {} failed to upgrade.",
                                             JobId_, upgraded_ ,failed_));
            delete this;
        }
    };
}