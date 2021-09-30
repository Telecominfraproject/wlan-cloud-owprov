//
// Created by stephane bourque on 2021-09-29.
//

#ifndef OWPROV_SDK_STUBS_H
#define OWPROV_SDK_STUBS_H

#include "OpenAPIRequest.h"

namespace OpenWifi::SDK {

    bool DeviceSetVenue(const std::string & SerialNumber, const std::string &uuid, Poco::JSON::Object::Ptr & Response);


};


#endif //OWPROV_SDK_STUBS_H
