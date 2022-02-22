//
// Created by stephane bourque on 2022-02-22.
//

#include "Signup.h"

namespace OpenWifi {

    int Signup::Start() {
        GracePeriod_ = MicroService::instance().ConfigGetInt("signup.graceperiod", 60*60);
        LingerPeriod_ = MicroService::instance().ConfigGetInt("signup.lingerperiod", 24*60*60);

        return 0;
    }

    void Signup::Stop() {

    }

}