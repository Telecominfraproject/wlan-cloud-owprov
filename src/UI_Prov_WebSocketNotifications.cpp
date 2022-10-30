//
// Created by stephane bourque on 2022-10-29.
//

#include "UI_Prov_WebSocketNotifications.h"


namespace OpenWifi {

    void WebSocketNotificationJobContent::to_json(Poco::JSON::Object &Obj) const {
        RESTAPI_utils::field_to_json(Obj,"title",title);
        RESTAPI_utils::field_to_json(Obj,"jobId",jobId);
        RESTAPI_utils::field_to_json(Obj,"success",success);
        RESTAPI_utils::field_to_json(Obj,"error",error);
        RESTAPI_utils::field_to_json(Obj,"warning",warning);
        RESTAPI_utils::field_to_json(Obj,"timeStamp",timeStamp);
        RESTAPI_utils::field_to_json(Obj,"details",details);
    }

    bool WebSocketNotificationJobContent::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            RESTAPI_utils::field_from_json(Obj,"title",title);
            RESTAPI_utils::field_from_json(Obj,"jobId",jobId);
            RESTAPI_utils::field_from_json(Obj,"success",success);
            RESTAPI_utils::field_from_json(Obj,"error",error);
            RESTAPI_utils::field_from_json(Obj,"warning",warning);
            RESTAPI_utils::field_from_json(Obj,"timeStamp",timeStamp);
            RESTAPI_utils::field_from_json(Obj,"details",details);
            return true;
        } catch(...) {

        }
        return false;
    }

    void WebSocketNotificationRebootList::to_json(Poco::JSON::Object &Obj) const {
        RESTAPI_utils::field_to_json(Obj,"title",title);
        RESTAPI_utils::field_to_json(Obj,"jobId",jobId);
        RESTAPI_utils::field_to_json(Obj,"success",success);
        RESTAPI_utils::field_to_json(Obj,"warning",warning);
        RESTAPI_utils::field_to_json(Obj,"timeStamp",timeStamp);
        RESTAPI_utils::field_to_json(Obj,"details",details);
    }

    bool WebSocketNotificationRebootList::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            RESTAPI_utils::field_from_json(Obj,"title",title);
            RESTAPI_utils::field_from_json(Obj,"jobId",jobId);
            RESTAPI_utils::field_from_json(Obj,"success",success);
            RESTAPI_utils::field_from_json(Obj,"warning",warning);
            RESTAPI_utils::field_from_json(Obj,"timeStamp",timeStamp);
            RESTAPI_utils::field_from_json(Obj,"details",details);
            return true;
        } catch(...) {

        }
        return false;
    }

    void WebSocketNotificationUpgradeList::to_json(Poco::JSON::Object &Obj) const {
        RESTAPI_utils::field_to_json(Obj,"title",title);
        RESTAPI_utils::field_to_json(Obj,"jobId",jobId);
        RESTAPI_utils::field_to_json(Obj,"success",success);
        RESTAPI_utils::field_to_json(Obj,"notConnected",not_connected);
        RESTAPI_utils::field_to_json(Obj,"noFirmware",no_firmware);
        RESTAPI_utils::field_to_json(Obj,"skipped",skipped);
        RESTAPI_utils::field_to_json(Obj,"timeStamp",timeStamp);
        RESTAPI_utils::field_to_json(Obj,"details",details);
    }

    bool WebSocketNotificationUpgradeList::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            RESTAPI_utils::field_from_json(Obj,"title",title);
            RESTAPI_utils::field_from_json(Obj,"jobId",jobId);
            RESTAPI_utils::field_from_json(Obj,"success",success);
            RESTAPI_utils::field_from_json(Obj,"notConnected",not_connected);
            RESTAPI_utils::field_from_json(Obj,"noFirmware",no_firmware);
            RESTAPI_utils::field_from_json(Obj,"skipped",skipped);
            RESTAPI_utils::field_from_json(Obj,"timeStamp",timeStamp);
            RESTAPI_utils::field_from_json(Obj,"details",details);
            return true;
        } catch(...) {

        }
        return false;
    }

    void WebSocketClientNotificationVenueUpdateJobCompletionToUser( WebSocketClientNotificationVenueUpdateJob_t &N) {
        N.type = "venue_configuration_update";
        UI_WebSocketClientServer()->SendNotification(N);
    }

    void WebSocketClientNotificationVenueRebootCompletionToUser( WebSocketClientNotificationVenueRebootList_t &N) {
        N.type = "venue_rebooter";
        UI_WebSocketClientServer()->SendNotification(N);
    }

    void WebSocketClientNotificationVenueUpgradeCompletionToUser( WebSocketClientNotificationVenueUpgradeList_t &N) {
        N.type = "venue_upgrader";
        UI_WebSocketClientServer()->SendNotification(N);
    }

    void WebSocketClientNotificationVenueUpdateJobCompletionToUser( const std::string & User, WebSocketClientNotificationVenueUpdateJob_t &N) {
        N.type = "venue_configuration_update";
        UI_WebSocketClientServer()->SendUserNotification(User,N);
    }

    void WebSocketClientNotificationVenueRebootCompletionToUser( const std::string & User, WebSocketClientNotificationVenueRebootList_t &N) {
        N.type = "venue_rebooter";
        UI_WebSocketClientServer()->SendUserNotification(User,N);
    }

    void WebSocketClientNotificationVenueUpgradeCompletionToUser( const std::string & User, WebSocketClientNotificationVenueUpgradeList_t &N) {
        N.type = "venue_upgrader";
        UI_WebSocketClientServer()->SendUserNotification(User,N);
    }

} // namespace OpenWifi
