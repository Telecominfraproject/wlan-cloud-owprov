//
// Created by stephane bourque on 2022-04-01.
//

#pragma once

#include "framework/MicroService.h"
#include "StorageService.h"
#include "APConfig.h"
#include "sdks/SDK_gw.h"

namespace OpenWifi {

    static void GetRejectedLines(const Poco::JSON::Object::Ptr &Response, Types::StringVec & Warnings) {
        try {
            if(Response->has("results")) {
                auto Results = Response->get("results").extract<Poco::JSON::Object::Ptr>();
                auto Status = Results->get("status").extract<Poco::JSON::Object::Ptr>();
                auto Rejected = Status->getArray("rejected");
                std::transform(Rejected->begin(),Rejected->end(),std::back_inserter(Warnings), [](auto i) -> auto { return i.toString(); });
//                for(const auto &i:*Rejected)
                //                  Warnings.push_back(i.toString());
            }
        } catch (...) {
        }
    }


    class VenueConfigUpdater: public Poco::Runnable {
    public:
        explicit VenueConfigUpdater(const std::string & VenueUUID, const SecurityObjects::UserInfo &UI, uint64_t When, Poco::Logger &L) :
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

        inline Poco::Logger & Logger() { return Logger_; }

        inline void run() final {

            if(When_ && When_>OpenWifi::Now())
                Poco::Thread::trySleep( (long) (When_ - OpenWifi::Now()) * 1000 );

            Logger().information(fmt::format("Job {} Starting.", JobId_));

            ProvObjects::Venue  Venue;
            uint64_t Updated = 0, Failed = 0 , BadConfigs = 0 ;
            if(StorageService()->VenueDB().GetRecord("id",VenueUUID_,Venue)) {
                for(const Types::UUID_t &uuid:Venue.devices) {
                    ProvObjects::InventoryTag   Device;

                    if(StorageService()->InventoryDB().GetRecord("id",uuid,Device)) {
                        Logger().debug(fmt::format("{}: Computing configuration.",Device.serialNumber));
                        auto DeviceConfig = std::make_shared<APConfig>(Device.serialNumber, Device.deviceType, Logger(), false);
                        auto Configuration = Poco::makeShared<Poco::JSON::Object>();
                        Poco::JSON::Object ErrorsObj, WarningsObj;
                        ProvObjects::InventoryConfigApplyResult Results;
                        if (DeviceConfig->Get(Configuration)) {
                            std::ostringstream OS;
                            Configuration->stringify(OS);
                            Results.appliedConfiguration = OS.str();
                            auto Response=Poco::makeShared<Poco::JSON::Object>();
                            Logger().debug(fmt::format("{}: Sending configuration push.",Device.serialNumber));
                            if (SDK::GW::Device::Configure(nullptr, Device.serialNumber, Configuration, Response)) {
                                Logger().debug(fmt::format("{}: Sending configuration pushed.",Device.serialNumber));
                                GetRejectedLines(Response, Results.warnings);
                                Results.errorCode = 0;
                                Logger().information(fmt::format("Device {} updated.", Device.serialNumber));
                                Updated++;
                            } else {
                                Logger().information(fmt::format("Device {} was not updated.", Device.serialNumber));
                                Results.errorCode = 1;
                                Failed++;
                            }
                        } else {
                            Logger().debug(fmt::format("{}: Configuration is bad.",Device.serialNumber));
                            Results.errorCode = 1;
                            BadConfigs++;
                        }
                    }
                }
            } else {
                Logger().warning(fmt::format("Venue {} no longer exists.",VenueUUID_));
            }
            Logger().information(fmt::format("Job {} Completed: {} updated, {} failed to update{} , {} bad configurations.",
                                             JobId_, Updated ,Failed, BadConfigs));
            delete this;
        }
    };

}