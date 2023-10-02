//
// Created by stephane bourque on 2023-09-28.
//

#pragma once

#include <Poco/Net/IPAddress.h>
#include <Poco/Net/SocketAddress.h>
#include <framework/utils.h>

namespace OpenWifi {

    namespace Orion {
        static inline const std::vector<Utils::HostNameServerResult> OrionWifiServerAddresses = {
                { "216.239.32.91", 2083 },
                { "216.239.34.91", 2083 }
        };

        inline std::vector<Utils::HostNameServerResult> GetServers() {
            return OrionWifiServerAddresses;
        }

    }

}