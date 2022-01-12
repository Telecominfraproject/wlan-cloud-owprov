//
// Created by stephane bourque on 2022-01-11.
//

#pragma once

#include "framework/MicroService.h"
#include "RESTObjects/RESTAPI_SecurityObjects.h"

namespace OpenWifi::SDK::Sec {

    namespace User {
        bool Exists(RESTAPIHandler *client, const Types::UUID_t & User);
        bool Get(RESTAPIHandler *client, const Types::UUID_t & User, SecurityObjects::UserInfo & UserInfo);
    }

    namespace Subscriber {
        bool Exists(RESTAPIHandler *client, const Types::UUID_t & User);
        bool Get(RESTAPIHandler *client, const Types::UUID_t & User, SecurityObjects::UserInfo & UserInfo);
    }

}

