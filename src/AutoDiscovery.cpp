//
// Created by stephane bourque on 2021-09-29.
//

#include "AutoDiscovery.h"
#include "Poco/JSON/Parser.h"
#include "StorageService.h"
#include "framework/KafkaManager.h"
#include "framework/KafkaTopics.h"
#include "framework/ow_constants.h"

namespace OpenWifi {

	int AutoDiscovery::Start() {
		poco_information(Logger(), "Starting...");
		Running_ = true;
		Types::TopicNotifyFunction F = [this](const std::string &Key, const std::string &Payload) {
			this->ConnectionReceived(Key, Payload);
		};
		ConnectionWatcherId_ = KafkaManager()->RegisterTopicWatcher(KafkaTopics::CONNECTION, F);
		Worker_.start(*this);
		return 0;
	};

	void AutoDiscovery::Stop() {
		poco_information(Logger(), "Stopping...");
		Running_ = false;
		KafkaManager()->UnregisterTopicWatcher(KafkaTopics::CONNECTION, ConnectionWatcherId_);
		Queue_.wakeUpAll();
		Worker_.join();
		poco_information(Logger(), "Stopped...");
	};

	void AutoDiscovery::run() {
		Poco::AutoPtr<Poco::Notification> Note(Queue_.waitDequeueNotification());
		Utils::SetThreadName("auto-discovery");
		while (Note && Running_) {
			auto Msg = dynamic_cast<DiscoveryMessage *>(Note.get());
			if (Msg != nullptr) {
				try {
                    DBGLINE
					Poco::JSON::Parser Parser;
					auto Object = Parser.parse(Msg->Payload()).extract<Poco::JSON::Object::Ptr>();
                    DBGLINE

                    std::cout << Msg->Payload() << std::endl;
					if (Object->has(uCentralProtocol::PAYLOAD)) {
                        DBGLINE
                        auto PayloadObj = Object->getObject(uCentralProtocol::PAYLOAD);
                        if (PayloadObj->has("ping")) {
                            auto PingObj = PayloadObj->getObject("ping");
                            std::string ConnectedIP, SerialNumber, DeviceType;
                            DBGLINE
                            if (PayloadObj->has(uCentralProtocol::CONNECTIONIP))
                                DBGLINE
                            ConnectedIP =
                                    PayloadObj->get(uCentralProtocol::CONNECTIONIP).toString();
                            if (PayloadObj->has(uCentralProtocol::CAPABILITIES)) {
                                DBGLINE
                                auto CapObj = PayloadObj->getObject(uCentralProtocol::CAPABILITIES);
                                if (CapObj->has(uCentralProtocol::COMPATIBLE)) {
                                    DBGLINE
                                    DeviceType = CapObj->get(uCentralProtocol::COMPATIBLE).toString();
                                    SerialNumber = PayloadObj->get(uCentralProtocol::SERIAL).toString();
                                    DBGLINE
                                }
                            } else if (PayloadObj->has(uCentralProtocol::PING)) {
                                DBGLINE
                                auto PingMessage = PayloadObj->getObject(uCentralProtocol::PING);
                                DBGLINE
                                if (PingMessage->has(uCentralProtocol::FIRMWARE) &&
                                    PingMessage->has(uCentralProtocol::SERIALNUMBER) &&
                                    PingMessage->has(uCentralProtocol::COMPATIBLE)) {
                                    if (PingMessage->has(uCentralProtocol::CONNECTIONIP))
                                        ConnectedIP =
                                                PingMessage->get(uCentralProtocol::CONNECTIONIP).toString();
                                    SerialNumber =
                                            PingMessage->get(uCentralProtocol::SERIALNUMBER).toString();
                                    DeviceType =
                                            PingMessage->get(uCentralProtocol::COMPATIBLE).toString();
                                    DBGLINE
                                }
                                DBGLINE
                            }
                            std::string Locale;
                            if (PayloadObj->has("locale")) {
                                DBGLINE
                                Locale = PayloadObj->get("locale").toString();
                                DBGLINE
                            }

                            if (!SerialNumber.empty()) {
                                DBGLINE
                                StorageService()->InventoryDB().CreateFromConnection(
                                        SerialNumber, ConnectedIP, DeviceType, Locale);
                            }
                        }
                    }
				} catch (const Poco::Exception &E) {
                    DBGLINE
                    std::cout << "EX:" << Msg->Payload() << std::endl;
					Logger().log(E);
                    DBGLINE
				} catch (...) {
                    DBGLINE
				}
			} else {
                DBGLINE
			}
            DBGLINE
			Note = Queue_.waitDequeueNotification();
            DBGLINE
		}
	}

} // namespace OpenWifi