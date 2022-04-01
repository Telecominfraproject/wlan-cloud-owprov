//
// Created by stephane bourque on 2022-04-01.
//

#pragma once

#include "framework/MicroService.h"
#include "StorageService.h"

namespace OpenWifi {

    class VenueConfigUpdater: public Poco::Runnable {
    public:
        explicit VenueConfigUpdater(const std::string & VenueUUID, const SecurityObjects::UserInfo &UI, uint64_t When, Poco::Logger &L) :
            VenueUUID_(VenueUUID),
            UI_(UI),
            When_(When),
            Logger_(L)
        {

        }

        std::string Start() {
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

        inline Poco::Logger & Logger() { return Logger_; }

        void run() final {

            if(When_ && When_>OpenWifi::Now())
                Poco::Thread::trySleep( (long) (When_ - OpenWifi::Now()) * 1000 );

            Logger().information(fmt::format("Job {} Starting.", JobId_));

            ProvObjects::Venue  Venue;
            if(StorageService()->VenueDB().GetRecord("id",VenueUUID_,Venue)) {
                for(const Types::UUID_t &uuid:Venue.devices) {
                    std::cout << "Updating device: " << uuid << std::endl;
                }
            } else {
                Logger().warning(fmt::format("Venue {} no longer exists.",VenueUUID_));
            }

            Logger().information(fmt::format("Job {} Completed.", JobId_));
            delete this;
        }
    };

}