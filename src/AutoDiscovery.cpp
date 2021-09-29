//
// Created by stephane bourque on 2021-09-29.
//

#include "AutoDiscovery.h"
#include "uCentralProtocol.h"
#include "KafkaManager.h"
#include "Kafka_topics.h"

namespace OpenWifi {

    class AutoDiscovery *AutoDiscovery::instance_ = nullptr;

    void AutoDiscovery::run() {
        Running_ = true ;
        while(Running_) {
            Poco::Thread::trySleep(2000);

            if(!Running_)
                break;

            while(!NewConnections_.empty()) {
                if(!Running_)
                    break;

                Types::StringPair  S;
                {
                    std::lock_guard G(Mutex_);
                    S = NewConnections_.front();
                    NewConnections_.pop();
                }
                try {
                    Poco::JSON::Parser  Parser;
                    auto Object = Parser.parse(S.second).extract<Poco::JSON::Object::Ptr>();

                    if(Object->has(uCentralProtocol::PAYLOAD)) {
                        auto PayloadObj = Object->getObject(uCentralProtocol::PAYLOAD);
                        std::string ConnectedIP, SerialNumber, DeviceType;
                        if(PayloadObj->has(uCentralProtocol::CONNECTIONIP) )
                            ConnectedIP = PayloadObj->get(uCentralProtocol::CONNECTIONIP).toString();
                        if(PayloadObj->has(uCentralProtocol::CAPABILITIES)) {
                            auto CapObj = PayloadObj->getObject(uCentralProtocol::CAPABILITIES);
                            if(CapObj->has(uCentralProtocol::COMPATIBLE)) {
                                DeviceType = CapObj->get(uCentralProtocol::COMPATIBLE).toString();
                                SerialNumber = PayloadObj->get(uCentralProtocol::SERIAL).toString();
                            }
                        } else if(PayloadObj->has(uCentralProtocol::PING)) {
                            auto PingMessage = PayloadObj->getObject(uCentralProtocol::PING);
                            if( PingMessage->has(uCentralProtocol::FIRMWARE) &&
                                PingMessage->has(uCentralProtocol::SERIALNUMBER) &&
                                PingMessage->has(uCentralProtocol::COMPATIBLE)) {
                                SerialNumber = PingMessage->get( uCentralProtocol::SERIALNUMBER).toString();
                                DeviceType = PingMessage->get( uCentralProtocol::COMPATIBLE).toString();
                            }
                        }
                        if(!SerialNumber.empty()) {
                            std::cout << "SerialNUmber: " << SerialNumber << "  CID: " << ConnectedIP << " DeviceType: " << DeviceType << std::endl;
                        }
                    }
                } catch (const Poco::Exception &E) {
                    Logger_.log(E);
                }
            }
        }
    };

    int AutoDiscovery::Start() {
        Types::TopicNotifyFunction F = [this](std::string s1,std::string s2) { this->ConnectionReceived(s1,s2); };
        ConnectionWatcherId_ = KafkaManager()->RegisterTopicWatcher(KafkaTopics::CONNECTION, F);
        Worker_.start(*this);
        return 0;
    };

    void AutoDiscovery::Stop() {
        KafkaManager()->UnregisterTopicWatcher(KafkaTopics::CONNECTION, ConnectionWatcherId_);
        Running_ = false;
        Worker_.wakeUp();
        Worker_.join();
    };

    void AutoDiscovery::ConnectionReceived( const std::string & Key, const std::string & Message) {
        std::lock_guard G(Mutex_);
        NewConnections_.push(std::make_pair(Key,Message));
    }

}