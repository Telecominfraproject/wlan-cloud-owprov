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
#include "JobController.h"

namespace OpenWifi {
    class VenueDeviceUpgrade : public Poco::Runnable {
    public:
        VenueDeviceUpgrade(const std::string &UUID, const std::string &venue, const ProvObjects::DeviceRules &Rules, Poco::Logger &L) :
                uuid_(UUID),
                venue_(venue),
                rules_(Rules),
                Logger_(L) {
        }

        void run() final {
            ProvObjects::InventoryTag   Device;
            started_=true;
            if(StorageService()->InventoryDB().GetRecord("id",uuid_,Device)) {
                SerialNumber = Device.serialNumber;

                Storage::ApplyRules(rules_,Device.deviceRules);
                if(Device.deviceRules.firmwareUpgrade=="no") {
                    poco_debug(Logger(),fmt::format("Skipped Upgrade: {}", Device.serialNumber));
                    skipped_++;
                    done_=true;
                    return;
                }

                FMSObjects::Firmware    F;
                if(SDK::FMS::Firmware::GetLatest(Device.deviceType,Device.deviceRules.rcOnly=="yes",F)) {
                    if (SDK::GW::Device::Upgrade(nullptr, Device.serialNumber, 0, F.uri)) {
                        Logger().debug(fmt::format("{}: Upgraded.",Device.serialNumber));
                        upgraded_++;
                    } else {
                        Logger().information(fmt::format("{}: Not Upgraded.", Device.serialNumber));
                        failed_++;
                    }
                } else {
                    Logger().information(fmt::format("{}: Not Upgraded. No firmware available.", Device.serialNumber));
                    failed_++;
                }
            }
            done_ = true;
            // std::cout << "Done push for " << Device.serialNumber << std::endl;
        }

        uint64_t        upgraded_=0, failed_=0, skipped_=0;
        bool            started_ = false,
                        done_ = false;
        std::string     SerialNumber;

    private:
        std::string     uuid_;
        std::string     venue_;
        ProvObjects::DeviceRules    rules_;
        Poco::Logger    &Logger_;
        inline Poco::Logger & Logger() { return Logger_; }
    };

    class VenueUpgrade: public Job {
    public:
        VenueUpgrade(const std::string &JobID, const std::string &name, const std::vector<std::string> & parameters, uint64_t when, const SecurityObjects::UserInfo &UI, Poco::Logger &L) :
                Job(JobID, name, parameters, when, UI, L) {

        }

        inline virtual void run() final {

            Utils::SetThreadName("venue-upgr");
            auto VenueUUID_ = Parameter(0);

            WebSocketClientNotificationVenueRebootList_t        N;

            ProvObjects::Venue  Venue;
            uint64_t upgraded_ = 0, failed_ = 0;
            if(StorageService()->VenueDB().GetRecord("id",VenueUUID_,Venue)) {

                N.content.title = fmt::format("Upgrading {} devices.", Venue.info.name);
                N.content.jobId = JobId();

                Poco::ThreadPool                Pool_;
                std::list<VenueDeviceUpgrade*>  JobList;
                ProvObjects::DeviceRules        Rules;

                StorageService()->VenueDB().EvaluateDeviceRules(Venue.info.id, Rules);

                for(const auto &uuid:Venue.devices) {
                    auto NewTask = new VenueDeviceUpgrade(uuid, Venue.info.name, Rules, Logger());
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
                            else
                                N.content.warning.push_back(current_job->SerialNumber);
                            upgraded_ += current_job->upgraded_;
                            failed_ += current_job->failed_;
                            job_it = JobList.erase(job_it);
                            delete current_job;
                        } else {
                            ++job_it;
                        }
                    }
                }

                Logger().debug("Waiting for outstanding upgrade threads to finish.");
                Pool_.joinAll();
                for(auto job_it = JobList.begin(); job_it !=JobList.end();) {
                    VenueDeviceUpgrade * current_job = *job_it;
                    if(current_job!= nullptr && current_job->done_) {
                        if(current_job->upgraded_)
                            N.content.success.push_back(current_job->SerialNumber);
                        else
                            N.content.warning.push_back(current_job->SerialNumber);
                        upgraded_ += current_job->upgraded_;
                        failed_ += current_job->failed_;
                        job_it = JobList.erase(job_it);
                        delete current_job;
                    } else {
                        ++job_it;
                    }
                }

                N.content.details = fmt::format("Job {} Completed: {} upgraded, {} failed to upgrade.",
                                                JobId(), upgraded_ ,failed_);
            } else {
                N.content.details = fmt::format("Venue {} no longer exists.",VenueUUID_);
                Logger().warning(N.content.details);
            }

            // std::cout << N.content.details << std::endl;
            WebSocketClientNotificationVenueRebootCompletionToUser(UserInfo().email,N);
            Logger().information(fmt::format("Job {} Completed: {} upgraded, {} failed to upgrade.",
                                             JobId(), upgraded_ ,failed_));
            Utils::SetThreadName("free");

            Complete();
        }
    };
}