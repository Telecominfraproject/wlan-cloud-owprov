//
// Created by stephane bourque on 2021-09-29.
//

#ifndef OWPROV_AUTODISCOVERY_H
#define OWPROV_AUTODISCOVERY_H

#include "SubSystemServer.h"
#include "OpenWifiTypes.h"

namespace OpenWifi {
    class AutoDiscovery : public SubSystemServer, Poco::Runnable {
    public:

        static AutoDiscovery *instance() {
            if(instance_== nullptr)
                instance_ = new AutoDiscovery;
            return instance_;
        }

        int Start() override;
        void Stop() override;
        void run() override;
        void ConnectionReceived( const std::string & Key, const std::string & Message);
        [[nodiscard]] const std::string & firmwareUpgrade() { return firmwareUpgrade_; }
        bool firmwareRCOnly() const { return firmwareRCOnly_; }

    private:
        static AutoDiscovery 		*instance_;
        Poco::Thread                Worker_;
        std::atomic_bool            Running_ = false;
        int                         ConnectionWatcherId_=0;
        Types::StringPairQueue      NewConnections_;
        std::string                 firmwareUpgrade_{"no"};
        bool                        firmwareRCOnly_=false;

        AutoDiscovery() noexcept:
            SubSystemServer("AutoDiscovery", "AUTO-DISCOVERY", "discovery")
            {
            }
    };

    inline AutoDiscovery * AutoDiscovery() { return AutoDiscovery::instance(); }

}

#endif //OWPROV_AUTODISCOVERY_H
