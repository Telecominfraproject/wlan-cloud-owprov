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

    void AutoDiscovery::run() {
        Logger().information("Starting...");
        Poco::AutoPtr<Poco::Notification>	Note(Queue_.waitDequeueNotification());
        Logger().information("Entering loop...");
        while(Note && Running_) {
            _OWDEBUG_
            Logger().information("Waiting for device message...");
            _OWDEBUG_
            auto Msg = dynamic_cast<DiscoveryMessage *>(Note.get());
            _OWDEBUG_
            if(Msg!= nullptr) {
                _OWDEBUG_
                Logger().information("Message received...");
                _OWDEBUG_
                try {
                    _OWDEBUG_
                    Poco::JSON::Parser Parser;
                    auto Object = Parser.parse(Msg->Payload()).extract<Poco::JSON::Object::Ptr>();
                    _OWDEBUG_

                    if (Object->has(uCentralProtocol::PAYLOAD)) {
                        _OWDEBUG_
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
                        _OWDEBUG_
                        if (!SerialNumber.empty()) {
                            _OWDEBUG_
                            StorageService()->InventoryDB().CreateFromConnection(SerialNumber, ConnectedIP, DeviceType);
                            _OWDEBUG_
                        }
                        _OWDEBUG_
                    }
                } catch (const Poco::Exception &E) {
                    _OWDEBUG_
                    Logger().log(E);
                    _OWDEBUG_
                }
            } else {
                _OWDEBUG_
                Logger().information("No Message received...");
                _OWDEBUG_
            }
            _OWDEBUG_
            Note = Queue_.waitDequeueNotification();
            _OWDEBUG_
        }
        _OWDEBUG_
        Logger().information("Exiting...");
        _OWDEBUG_
    }

}