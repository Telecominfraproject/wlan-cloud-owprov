//
// Created by stephane bourque on 2021-09-29.
//

#include "AutoDiscovery.h"
#include "framework/ow_constants.h"
#include "framework/KafkaTopics.h"
#include "framework/KafkaManager.h"
#include "StorageService.h"
#include "Poco/JSON/Parser.h"

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
        Poco::AutoPtr<Poco::Notification>	Note(Queue_.waitDequeueNotification());
        Utils::SetThreadName("auto-discovery");
        while(Note && Running_) {
            auto Msg = dynamic_cast<DiscoveryMessage *>(Note.get());
            if(Msg!= nullptr) {
                try {
                    Poco::JSON::Parser Parser;
                    auto Object = Parser.parse(Msg->Payload()).extract<Poco::JSON::Object::Ptr>();

                    if (Object->has(uCentralProtocol::PAYLOAD)) {
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
                        std::string Locale;
                        if(PayloadObj->has("locale"))
                            Locale = PayloadObj->get("locale").toString();

                        if (!SerialNumber.empty()) {
                            StorageService()->InventoryDB().CreateFromConnection(SerialNumber, ConnectedIP, DeviceType, Locale);
                        }
                    }
                } catch (const Poco::Exception &E) {
                    Logger().log(E);
                } catch (...) {

                }
            } else {

            }
            Note = Queue_.waitDequeueNotification();
        }
    }

}