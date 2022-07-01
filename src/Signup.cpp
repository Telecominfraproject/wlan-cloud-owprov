//
// Created by stephane bourque on 2022-02-22.
//

#include "Signup.h"
#include "StorageService.h"
#include "sdks/SDK_gw.h"

namespace OpenWifi {

    int Signup::Start() {
        GracePeriod_ = MicroService::instance().ConfigGetInt("signup.graceperiod", 60*60);
        LingerPeriod_ = MicroService::instance().ConfigGetInt("signup.lingerperiod", 24*60*60);

        SignupDB::RecordVec Signups_;
        StorageService()->SignupDB().GetIncompleteSignups(Signups_);

        for(const auto &i:Signups_) {
            OutstandingSignups_[i.info.id] = i;
        }

        TimerCallback_ = std::make_unique<Poco::TimerCallback<Signup>>(*this,&Signup::onTimer);
        Timer_.setStartInterval( 60 * 1000);            // first run in 20 seconds
        Timer_.setPeriodicInterval(1 * 60 * 60 * 1000); // 1 hours
        Timer_.start(*TimerCallback_);

        Worker_.start(*this);
        return 0;
    }

    void Signup::Stop() {
        Running_ = false;
        Timer_.stop();
        Worker_.wakeUp();
        Worker_.join();
    }

    void Signup::onTimer([[maybe_unused]] Poco::Timer &timer) {
        std::lock_guard     G(Mutex_);
        StorageService()->SignupDB().RemoveIncompleteSignups();
    }

    void Signup::run() {
        Running_ = true;
        Utils::SetThreadName("signup-mgr");
        while(Running_) {
            Poco::Thread::trySleep(5000);

            if(!Running_)
                break;

            //  get the list of outstanding stuff and see if we have gotten the device...
            std::lock_guard     G(Mutex_);
            for(auto &[uuid,SE]:OutstandingSignups_) {
                if(SE.statusCode == ProvObjects::SignupStatusCodes::SignupWaitingForDevice) {
                    //  look for the device...
                    ProvObjects::InventoryTag   IT;
                    auto TrySerialNumber = Utils::SerialNumberToInt(SE.macAddress);
                    for(uint i=0;i<4;++i) {
                        std::string SerialNumber = Utils::IntToSerialNumber(TrySerialNumber+i);
                        if (StorageService()->InventoryDB().GetRecord("serialNumber", SerialNumber, IT)) {
                            //  if the deviceType is empty, we know that the device has not contacted
                            //  the service yet.
                            if (IT.deviceType.empty())
                                continue;

                            //  We have a device type, so the device contacted us.
                            //  We must now complete the device transfer to this new subscriber and complete the
                            //  signup job.
                            IT.subscriber = SE.userId;
                            IT.info.modified = OpenWifi::Now();
                            IT.realMacAddress = SE.macAddress;
                            if(IT.entity.empty()) {

                            } else {
                                // if the entity was not a subscriber entity, then we need to goto the default entity
                            }
                            Poco::JSON::Object NewState;
                            NewState.set("method", "signup");
                            NewState.set("date", OpenWifi::Now());
                            NewState.set("status", "completed");
                            std::ostringstream OS;
                            NewState.stringify(OS);
                            IT.state = OS.str();
                            StorageService()->InventoryDB().UpdateRecord("id", IT.info.id, IT);

                            // we need to move this device to the SubscriberDevice DB
                            ProvObjects::SubscriberDevice   SD;
                            SD.info.id = MicroService::CreateUUID();
                            SD.info.modified = SD.info.created = OpenWifi::Now();
                            SD.info.name = IT.realMacAddress;
                            SD.operatorId = SE.operatorId;
                            SD.serialNumber = SerialNumber;
                            SD.realMacAddress = SE.macAddress;
                            SD.locale = IT.locale;
                            SD.deviceType = IT.deviceType;
                            SD.state = OS.str();
                            SD.subscriberId = SE.userId;

                            Logger().information(fmt::format("Setting service class for {}",SD.serialNumber));
                            SD.serviceClass = StorageService()->ServiceClassDB().DefaultForOperator(SE.operatorId);
                            Logger().information(fmt::format("Removing old device information for {}",SD.serialNumber));
                            StorageService()->SubscriberDeviceDB().DeleteRecord("serialNumber", SD.serialNumber);
                            Logger().information(fmt::format("Creating subscriber device for {}",SD.serialNumber));
                            StorageService()->SubscriberDeviceDB().CreateRecord(SD);
                            Logger().information(fmt::format("Removing old inventory for {}",SD.serialNumber));
                            StorageService()->InventoryDB().DeleteRecord("serialNumber", SD.serialNumber);

                            SE.status = "signup completed";
                            SE.serialNumber = SerialNumber;
                            SE.statusCode = ProvObjects::SignupStatusCodes::SignupSuccess;
                            SE.completed = OpenWifi::Now();
                            SE.info.modified = OpenWifi::Now();
                            SE.error = 0;
                            Logger().information(fmt::format("Completed signup for {}",SD.serialNumber));
                            StorageService()->SignupDB().UpdateRecord("id", SE.info.id, SE);
                            Logger().information(fmt::format("Setting GW subscriber for {}",SD.serialNumber));
                            SDK::GW::Device::SetSubscriber(SerialNumber, IT.subscriber);
                            Logger().information(fmt::format("Success for {}",SD.serialNumber));
                            break;
                        }
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