//
// Created by stephane bourque on 2022-02-23.
//

#pragma once

#include <set>

#include "framework/MicroService.h"
#include "Poco/Timer.h"

namespace OpenWifi {

    class DeviceTypeCache : public SubSystemServer {
    public:
        static auto instance() {
            static auto instance_ = new DeviceTypeCache;
            return instance_;
        }

        inline int Start() final {
            InitializeCache();
            TimerCallback_ = std::make_unique<Poco::TimerCallback<DeviceTypeCache>>(*this,&DeviceTypeCache::onTimer);
            Timer_.setStartInterval( 60 * 1000);            // first run in 60 seconds
            Timer_.setPeriodicInterval(1 * 60 * 60 * 1000); // 1 hours
            Timer_.start(*TimerCallback_);
            return 0;
        }

        inline void Stop() final {
            Timer_.stop();
        }

        inline void onTimer(Poco::Timer & timer) {
            UpdateDeviceTypes();
        }

    private:
        std::atomic_bool                                        Initialized_=false;
        Poco::Timer                                             Timer_;
        std::set<std::string>                                   DeviceTypes_;
        std::unique_ptr<Poco::TimerCallback<DeviceTypeCache>>   TimerCallback_;

        inline DeviceTypeCache() noexcept:
                SubSystemServer("DeviceTypes", "DEV-TYPES", "devicetypes")
        {
        }

        void InitializeCache() {
            std::lock_guard G(Mutex_);

            Initialized_ = true;
            std::string DeviceTypes;
            if(AppServiceRegistry().Get("deviceTypes",DeviceTypes)) {
                Poco::JSON::Parser  P;
                try {
                    auto O = P.parse(DeviceTypes).extract<Poco::JSON::Array::Ptr>();
                    for(const auto &i:*O) {
                        DeviceTypes_.insert(i.toString());
                    }
                } catch (...) {

                }
            }
        }

        bool UpdateDeviceTypes() {
            try {
                Types::StringPairVec QueryData;

                QueryData.push_back(std::make_pair("deviceSet","true"));
                OpenAPIRequestGet	Req(    uSERVICE_FIRMWARE,
                                             "/api/v1/firmwares",
                                             QueryData,
                                             10000);

                Poco::JSON::Object::Ptr Response;
                auto StatusCode = Req.Do(Response);
                if( StatusCode == Poco::Net::HTTPResponse::HTTP_OK) {
                    if(Response->isArray("deviceTypes")) {
                        std::lock_guard G(Mutex_);
                        DeviceTypes_.clear();
                        auto Array = Response->getArray("deviceTypes");
                        for(const auto &i:*Array) {
                            std::cout << "Adding deviceType:" << i.toString() << std::endl;
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

        void SaveCache() {
            std::lock_guard G(Mutex_);

            Poco::JSON::Array   Arr;
            for(auto const &i:DeviceTypes_)
                Arr.add(i);

            std::stringstream OS;
            Arr.stringify(OS);

            AppServiceRegistry().Set("deviceTypes", OS.str());
        }
    };

    auto DeviceTypeCache() { return DeviceTypeCache::instance(); }

}