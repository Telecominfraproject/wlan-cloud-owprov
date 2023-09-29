//
// Created by stephane bourque on 2023-09-28.
//

#pragma once

#include <Poco/Net/IPAddress.h>
#include <Poco/Net/SocketAddress.h>

namespace OpenWifi {

    namespace Orion {
        inline const std::vector<Poco::Net::SocketAddress> ServerAddresses = {
                Poco::Net::SocketAddress("216.239.32.91:2083"),
                Poco::Net::SocketAddress("216.239.34.91:2083")
        };
    }

}