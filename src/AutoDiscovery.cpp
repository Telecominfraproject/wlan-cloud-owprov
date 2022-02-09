//
// Created by stephane bourque on 2021-09-29.
//

#include "AutoDiscovery.h"
#include "framework/uCentral_Protocol.h"
#include "framework/KafkaTopics.h"
#include "StorageService.h"

namespace OpenWifi {

    int AutoDiscovery::Start() {
        Running_ = true;
        Types::TopicNotifyFunction F = [this](const std::string &Key, const std::string &Payload) { this->ConnectionReceived(Key,Payload); };
        ConnectionWatcherId_ = KafkaManager()->RegisterTopicWatcher(KafkaTopics::CONNECTION, F);
        Worker_.start(*this);
        return 0;
    };

    void AutoDiscovery::Stop() {
        Running_ = false;
        KafkaManager()->UnregisterTopicWatcher(KafkaTopics::CONNECTION, ConnectionWatcherId_);
        Queue_.wakeUpAll();
        Worker_.join();
    };

#define __DBG__ std::cout << __FILE__ << ": " << __LINE__ << std::endl;

    void AutoDiscovery::run() {
        Logger().information("Starting...");
        Poco::AutoPtr<Poco::Notification>	Note(Queue_.waitDequeueNotification());
        Logger().information("Entering loop...");
        while(Note && Running_) {
            __DBG__
            Logger().information("Waiting for device message...");
            __DBG__
            auto Msg = dynamic_cast<DiscoveryMessage *>(Note.get());
            __DBG__
            if(Msg!= nullptr) {
                __DBG__
                Logger().information("Message received...");
                __DBG__
                try {
                    __DBG__
                    Poco::JSON::Parser Parser;
                    auto Object = Parser.parse(Msg->Payload()).extract<Poco::JSON::Object::Ptr>();
                    __DBG__

                    if (Object->has(uCentralProtocol::PAYLOAD)) {
                        __DBG__
                        auto PayloadObj = Object->getObject(uCentralProtocol::PAYLOAD);
                        std::string ConnectedIP, SerialNumber, DeviceType;
                        if (PayloadObj->has(uCentralProtocol::CONNECTIONIP))
                            ConnectedIP = PayloadObj->get(uCentralProtocol::CONNECTIONIP).toString();
                        if (PayloadObj->has(uCentralProtocol::CAPABILITIES)) {
                            auto CapObj = PayloadObj->getObject(uCentralProtocol::CAPABILITIES);
                            if (CapObj->has(uCentralProtocol::COMPATIBLE)) {
                                DeviceType = CapObj->get(uCentralProtocol::COMPATIBLE).toString();
                                SerialNumber = PayloadObj->get(uCentralProtocol::SERIAL).toString();
                            }
                        } else if (PayloadObj->has(uCentralProtocol::PING)) {
                            auto PingMessage = PayloadObj->getObject(uCentralProtocol::PING);
                            if (PingMessage->has(uCentralProtocol::FIRMWARE) &&
                                PingMessage->has(uCentralProtocol::SERIALNUMBER) &&
                                PingMessage->has(uCentralProtocol::COMPATIBLE)) {
                                if (PingMessage->has(uCentralProtocol::CONNECTIONIP))
                                    ConnectedIP = PingMessage->get(uCentralProtocol::CONNECTIONIP).toString();
                                SerialNumber = PingMessage->get(uCentralProtocol::SERIALNUMBER).toString();
                                DeviceType = PingMessage->get(uCentralProtocol::COMPATIBLE).toString();
                            }
                        }
                        __DBG__
                        if (!SerialNumber.empty()) {
                            __DBG__
                            StorageService()->InventoryDB().CreateFromConnection(SerialNumber, ConnectedIP, DeviceType);
                            __DBG__
                        }
                        __DBG__
                    }
                } catch (const Poco::Exception &E) {
                    __DBG__
                    Logger().log(E);
                    __DBG__
                }
            } else {
                __DBG__
                Logger().information("No Message received...");
                __DBG__
            }
            __DBG__
            Note = Queue_.waitDequeueNotification();
            __DBG__
        }
        __DBG__
        Logger().information("Exiting...");
        __DBG__
    }

}