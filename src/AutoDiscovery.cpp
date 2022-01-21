//
// Created by stephane bourque on 2021-09-29.
//

#include "AutoDiscovery.h"
#include "framework/uCentral_Protocol.h"
#include "framework/KafkaTopics.h"
#include "storage/storage_inventory.h"
#include "StorageService.h"
#include "framework/MicroService.h"

namespace OpenWifi {

    int AutoDiscovery::Start() {
        Messages_->Readable_ += Poco::delegate(this,&AutoDiscovery::onConnection);
        Types::TopicNotifyFunction F = [this](std::string s1,std::string s2) { this->ConnectionReceived(s1,s2); };
        ConnectionWatcherId_ = KafkaManager()->RegisterTopicWatcher(KafkaTopics::CONNECTION, F);
        return 0;
    };

    void AutoDiscovery::Stop() {
        Messages_->Readable_ -= Poco::delegate( this, &AutoDiscovery::onConnection);
        KafkaManager()->UnregisterTopicWatcher(KafkaTopics::CONNECTION, ConnectionWatcherId_);
    };

    void AutoDiscovery::onConnection(bool &b) {
        if (b) {
            DiscoveryMessage    Msg;

            auto ValidMessage = Messages_->Read(Msg);
            if(ValidMessage) {
                try {
                    Poco::JSON::Parser Parser;
                    auto Object = Parser.parse(Msg.Payload).extract<Poco::JSON::Object::Ptr>();

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
                        if (!SerialNumber.empty()) {
                            // std::cout << "SerialNUmber: " << SerialNumber << "  CID: " << ConnectedIP << " DeviceType: " << DeviceType << std::endl;
                            StorageService()->InventoryDB().CreateFromConnection(SerialNumber, ConnectedIP, DeviceType);
                        }
                    }
                } catch (const Poco::Exception &E) {
                    Logger().log(E);
                }
            }
        }
    }

    void AutoDiscovery::ConnectionReceived( const std::string & Key, const std::string & Message) {
        std::lock_guard G(Mutex_);
        Logger().information(Poco::format("Connection(%s): New connection notification.", Key));
        Messages_->Write(DiscoveryMessage{Key,Message});
    }
}