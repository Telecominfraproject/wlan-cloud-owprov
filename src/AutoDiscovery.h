//
// Created by stephane bourque on 2021-09-29.
//

#pragma once

#include "framework/MicroService.h"
#include "framework/OpenWifiTypes.h"

namespace OpenWifi {
    class AutoDiscovery : public SubSystemServer {
    public:

        static auto instance() {
            static auto instance_ = new AutoDiscovery;
            return instance_;
        }

        int Start() override;
        void Stop() override;
        void ConnectionReceived( const std::string & Key, const std::string & Message);

        struct DiscoveryMessage {
            std::string     Key;
            std::string     Payload;
        };

        void onConnection(bool& b);

    private:
        int                                     ConnectionWatcherId_=0;
        std::unique_ptr<FIFO<DiscoveryMessage>>	Messages_=std::make_unique<FIFO<DiscoveryMessage>>(100);

        Types::StringPairQueue      NewConnections_;

        AutoDiscovery() noexcept:
            SubSystemServer("AutoDiscovery", "AUTO-DISCOVERY", "discovery")
            {
            }
    };

    inline auto AutoDiscovery() { return AutoDiscovery::instance(); }

}

