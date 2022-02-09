//
// Created by stephane bourque on 2021-09-29.
//

#pragma once

#include "framework/MicroService.h"
#include "framework/OpenWifiTypes.h"

namespace OpenWifi {

    class DiscoveryMessage : public Poco::Notification {
        public:
            explicit DiscoveryMessage(const std::string &Key, const std::string &Payload ) :
                Key_(Key),
                Payload_(Payload) {}
            const std::string & Key() { return Key_; }
            const std::string & Payload() { return Payload_; }
        private:
            std::string     Key_;
            std::string     Payload_;
    };

    class AutoDiscovery : public SubSystemServer, Poco::Runnable {
        public:

            static auto instance() {
                static auto instance_ = new AutoDiscovery;
                return instance_;
            }

            int Start() override;
            void Stop() override;
            void ConnectionReceived( const std::string & Key, const std::string & Payload) {
                std::lock_guard G(Mutex_);
                Logger().information(Poco::format("Device(%s): Connection/Ping message.", Key));
                Queue_.enqueueNotification( new DiscoveryMessage(Key,Payload));
            }
            void run() override;

        private:
            uint64_t                                ConnectionWatcherId_=0;
            Poco::NotificationQueue                 Queue_;
            Poco::Thread                            Worker_;
            std::atomic_bool                        Running_=false;

            AutoDiscovery() noexcept:
                SubSystemServer("AutoDiscovery", "AUTO-DISCOVERY", "discovery")
                {
                }
        };

    inline auto AutoDiscovery() { return AutoDiscovery::instance(); }

}

