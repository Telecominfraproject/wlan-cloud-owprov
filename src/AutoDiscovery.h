//
// Created by stephane bourque on 2021-09-29.
//

#ifndef OWPROV_AUTODISCOVERY_H
#define OWPROV_AUTODISCOVERY_H

#include "framework/MicroService.h"
#include "framework/OpenWifiTypes.h"

namespace OpenWifi {
    class AutoDiscovery : public SubSystemServer, Poco::Runnable {
    public:

        static AutoDiscovery *instance() {
            static AutoDiscovery * instance_ = new AutoDiscovery;
            return instance_;
        }

        int Start() override;
        void Stop() override;
        void run() override;
        void ConnectionReceived( const std::string & Key, const std::string & Message);

    private:
        Poco::Thread                Worker_;
        std::atomic_bool            Running_ = false;
        int                         ConnectionWatcherId_=0;
        Types::StringPairQueue      NewConnections_;

        AutoDiscovery() noexcept:
            SubSystemServer("AutoDiscovery", "AUTO-DISCOVERY", "discovery")
            {
            }
    };

    inline AutoDiscovery * AutoDiscovery() { return AutoDiscovery::instance(); }

}

#endif //OWPROV_AUTODISCOVERY_H
