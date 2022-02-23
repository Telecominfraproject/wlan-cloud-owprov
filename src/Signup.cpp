//
// Created by stephane bourque on 2022-02-22.
//

#include "Signup.h"
#include "StorageService.h"

namespace OpenWifi {

    int Signup::Start() {
        GracePeriod_ = MicroService::instance().ConfigGetInt("signup.graceperiod", 60*60);
        LingerPeriod_ = MicroService::instance().ConfigGetInt("signup.lingerperiod", 24*60*60);

        SignupDB::RecordVec Signups_;
        StorageService()->SignupDB().GetIncompleteSignups(Signups_);

        for(const auto &i:Signups_) {
            OutstandingSignups_[i.info.id] = i;
        }
        Worker_.start(*this);
        return 0;
    }

    void Signup::Stop() {
        Running_ = false;
        Worker_.wakeUp();
        Worker_.join();
    }

    void Signup::run() {
        Running_ = true;
        while(Running_) {
            Poco::Thread::trySleep(5000);

            if(!Running_)
                break;

            //  get the list of outstanding stuff and see if we have gotten the device...
            std::lock_guard     G(Mutex_);
            for(auto &[uuid,SE]:OutstandingSignups_) {
                if(SE.status == "emailVerified") {
                    //  look for the device...
                    ProvObjects::InventoryTag   IT;
                    if(StorageService()->InventoryDB().GetRecord("serialNumber",SE.serialNumber,IT)) {
                        //  if the deviceType is empty, we know that the device has not contacted
                        //  the service yet.
                        if(IT.deviceType.empty())
                            continue;

                        //  We have a device type, so the device contacted us.
                        //  We must now complete the device transfer to this new subscriber and complete the
                        //  signup job.
                        IT.subscriber = SE.userId;
                        IT.info.modified = OpenWifi::Now();
                        Poco::JSON::Object  NewState;
                        NewState.set("method","signup");
                        NewState.set("date", OpenWifi::Now());
                        NewState.set("status", "completed");
                        std::ostringstream OS;
                        NewState.stringify(OS);
                        IT.state = OS.str();
                        StorageService()->InventoryDB().UpdateRecord("id",IT.info.id,IT);

                        SE.status = "signup completed";
                        SE.completed = OpenWifi::Now();
                        SE.info.modified = OpenWifi::Now();
                        SE.error = 0 ;
                        StorageService()->SignupDB().UpdateRecord("id", SE.info.id, SE);
                    }
                }
            }

            for(auto i = begin(OutstandingSignups_); i!=end(OutstandingSignups_);) {
                if(i->second.completed!=0)
                    i = OutstandingSignups_.erase(i);
                else
                    i++;
            }
        }
    }
}