//
// Created by stephane bourque on 2022-10-25.
//

#pragma once

#include <map>
#include <string>

#include "Poco/Runnable.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/JSON/Object.h"
#include "Poco/Net/SocketNotification.h"

#include "RESTObjects/RESTAPI_SecurityObjects.h"
#include "framework/SubSystemServer.h"
#include "framework/UI_WebSocketClientNotifications.h"

namespace OpenWifi {

	class UI_WebSocketClient;

	class UI_WebSocketClientProcessor {
	  public:
		virtual void Processor(const Poco::JSON::Object::Ptr &O, std::string &Answer, bool &Done ) = 0;
	  private:
	};

    struct UI_WebSocketClientInfo {
        std::unique_ptr<Poco::Net::WebSocket>   WS_ = nullptr;
        std::string 				            Id_;
        std::string					            UserName_;
        bool 			                        Authenticated_ = false;
        bool				                    SocketRegistered_=false;
        SecurityObjects::UserInfoAndPolicy      UserInfo_;

        UI_WebSocketClientInfo(Poco::Net::WebSocket &WS, const std::string &Id, const std::string &username) {
            WS_ =  std::make_unique<Poco::Net::WebSocket>(WS);
            Id_ = Id;
            UserName_ = username;
        }
    };

	class UI_WebSocketClientServer : public SubSystemServer, Poco::Runnable {

	  public:
		static auto instance() {
			static auto instance_ = new UI_WebSocketClientServer;
			return instance_;
		}

		int Start() override;
		void Stop() override;
		void run() override;
		Poco::Net::SocketReactor & Reactor() { return Reactor_; }
		void NewClient(Poco::Net::WebSocket &WS, const std::string &Id, const std::string &UserName);
		void SetProcessor(UI_WebSocketClientProcessor *F);
		// void UnRegister(const std::string &Id);
		[[nodiscard]] inline bool GeoCodeEnabled() const { return GeoCodeEnabled_; }
		[[nodiscard]] inline std::string GoogleApiKey() const { return GoogleApiKey_; }
		// [[nodiscard]] bool Send(const std::string &Id, const std::string &Payload);

		template <typename T> bool
		SendUserNotification(const std::string &userName, const WebSocketNotification<T> &Notification) {

			Poco::JSON::Object  Payload;
			Notification.to_json(Payload);
			Poco::JSON::Object  Msg;
			Msg.set("notification",Payload);
			std::ostringstream OO;
			Msg.stringify(OO);

			return SendToUser(userName,OO.str());
		}

		template <typename T> void SendNotification(const WebSocketNotification<T> &Notification) {
			Poco::JSON::Object  Payload;
			Notification.to_json(Payload);
			Poco::JSON::Object  Msg;
			Msg.set("notification",Payload);
			std::ostringstream OO;
			Msg.stringify(OO);
			SendToAll(OO.str());
		}

		[[nodiscard]] bool SendToUser(const std::string &userName, const std::string &Payload);
		void SendToAll(const std::string &Payload);

        using ClientList = std::map<int,std::unique_ptr<UI_WebSocketClientInfo>>;
    private:
		mutable std::atomic_bool Running_ = false;
		Poco::Thread 								Thr_;
		Poco::Net::SocketReactor					Reactor_;
		Poco::Thread								ReactorThread_;
        std::recursive_mutex                        LocalMutex_;
		bool GeoCodeEnabled_ = false;
		std::string GoogleApiKey_;
        ClientList    Clients_;
		UI_WebSocketClientProcessor                 *Processor_ = nullptr;
		UI_WebSocketClientServer() noexcept;
        void EndConnection(std::lock_guard<std::recursive_mutex> &G, ClientList::iterator & Client);

        void OnSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf);
        void OnSocketShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification> &pNf);
        void OnSocketError(const Poco::AutoPtr<Poco::Net::ErrorNotification> &pNf);

        ClientList::iterator FindWSClient( std::lock_guard<std::recursive_mutex> &G, int ClientSocket);


	};

	inline auto UI_WebSocketClientServer() { return UI_WebSocketClientServer::instance(); }

};
