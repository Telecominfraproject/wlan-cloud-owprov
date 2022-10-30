//
// Created by stephane bourque on 2022-10-29.
//

#pragma once

#include "framework/UI_WebSocketClientNotifications.h"
#include "framework/UI_WebSocketClientServer.h"

namespace OpenWifi {
    struct WebSocketNotificationJobContent {
        std::string                 title,
                details,
                jobId;
        std::vector<std::string>    success,
                error,
                warning;
        uint64_t                    timeStamp=OpenWifi::Utils::Now();

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    typedef WebSocketNotification<WebSocketNotificationJobContent>  WebSocketClientNotificationVenueUpdateJob_t;

    struct WebSocketNotificationRebootList {
        std::string                 title,
                details,
                jobId;
        std::vector<std::string>    success,
                warning;
        uint64_t                    timeStamp=OpenWifi::Utils::Now();

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    typedef WebSocketNotification<WebSocketNotificationRebootList> WebSocketClientNotificationVenueRebootList_t;

    struct WebSocketNotificationUpgradeList {
        std::string title,
                details,
                jobId;
        std::vector<std::string> success,
                skipped,
                no_firmware,
                not_connected;
        uint64_t timeStamp = OpenWifi::Utils::Now();

        void to_json(Poco::JSON::Object &Obj) const;

        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    typedef WebSocketNotification<WebSocketNotificationUpgradeList> WebSocketClientNotificationVenueUpgradeList_t;

    void WebSocketClientNotificationVenueUpdateJobCompletionToUser( const std::string & User, WebSocketClientNotificationVenueUpdateJob_t &N);
    void WebSocketClientNotificationVenueRebootCompletionToUser( const std::string & User, WebSocketClientNotificationVenueRebootList_t &N);
    void WebSocketClientNotificationVenueUpgradeCompletionToUser( const std::string & User, WebSocketClientNotificationVenueUpgradeList_t &N);
    void WebSocketClientNotificationVenueUpdateJobCompletionToUser( WebSocketClientNotificationVenueUpdateJob_t &N);
    void WebSocketClientNotificationVenueRebootCompletionToUser( WebSocketClientNotificationVenueRebootList_t &N);
    void WebSocketClientNotificationVenueUpgradeCompletionToUser( WebSocketClientNotificationVenueUpgradeList_t &N);
}
// namespace OpenWifi
