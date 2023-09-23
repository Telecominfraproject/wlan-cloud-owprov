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

    void AutoDiscovery::ProcessPing(const Poco::JSON::Object::Ptr & P, std::string &FW, std::string &SN,
                                    std::string &Compat, std::string &Conn, std::string &locale) {
        if (P->has(uCentralProtocol::CONNECTIONIP))
            Conn = P->get(uCentralProtocol::CONNECTIONIP).toString();
        if (P->has(uCentralProtocol::FIRMWARE))
            FW = P->get(uCentralProtocol::FIRMWARE).toString();
        if (P->has(uCentralProtocol::SERIALNUMBER))
            SN = P->get(uCentralProtocol::SERIALNUMBER).toString();
        if (P->has(uCentralProtocol::COMPATIBLE))
            Compat = P->get(uCentralProtocol::COMPATIBLE).toString();
        if (P->has("locale")) {
            locale = P->get("locale").toString();
        }
    }

    void AutoDiscovery::ProcessConnect(const Poco::JSON::Object::Ptr &P, std::string &FW, std::string &SN,
                                       std::string &Compat, std::string &Conn, std::string &locale) {
        if (P->has(uCentralProtocol::CONNECTIONIP))
            Conn = P->get(uCentralProtocol::CONNECTIONIP).toString();
        if (P->has(uCentralProtocol::FIRMWARE))
            FW = P->get(uCentralProtocol::FIRMWARE).toString();
        if (P->has(uCentralProtocol::SERIALNUMBER))
            SN = P->get(uCentralProtocol::SERIALNUMBER).toString();
        if (P->has("locale")) {
            locale = P->get("locale").toString();
        }
        if(P->has(uCentralProtocol::CAPABILITIES)) {
            auto CapObj = P->getObject(uCentralProtocol::CAPABILITIES);
            if (CapObj->has(uCentralProtocol::COMPATIBLE))
                Compat = CapObj->get(uCentralProtocol::COMPATIBLE).toString();
        }
    }

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
                        std::string ConnectedIP, SerialNumber, Compatible, Firmware, Locale ;
                        if (PayloadObj->has(uCentralProtocol::PING)) {
                            auto PingObj = PayloadObj->getObject("ping");
                            ProcessPing(PingObj, Firmware, SerialNumber, Compatible, ConnectedIP, Locale);
                        } else if(PayloadObj->has("capabilities")) {
                            ProcessConnect(PayloadObj, Firmware, SerialNumber, Compatible, ConnectedIP, Locale);
                        }

                        if (!SerialNumber.empty()) {
                            DBGLINE
                            StorageService()->InventoryDB().CreateFromConnection(
                                    SerialNumber, ConnectedIP, Compatible, Locale);
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