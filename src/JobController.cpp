//
// Created by stephane bourque on 2021-10-28.
//

#include "JobController.h"

namespace OpenWifi {

    void RegisterJobTypes();

    int JobController::Start() {

        RegisterJobTypes();

        if(!Running_)
            Thr_.start(*this);

        return 0;
    }

    void JobController::Stop() {
        if(Running_) {
            Running_ = false;
            Thr_.join();
        }
    }

    void JobController::run() {
        Running_ = true ;

        while(Running_) {
            Poco::Thread::trySleep(2000);
        }

    }

}