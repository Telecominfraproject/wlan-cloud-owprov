//
// Created by stephane bourque on 2021-11-01.
//

#pragma once

#include "framework/MicroService.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Environment.h"
#include "Poco/NObserver.h"
#include "Poco/Net/SocketNotification.h"

#include "RESTObjects/RESTAPI_SecurityObjects.h"
#include "RESTObjects/RESTAPI_ProvObjects.h"

namespace OpenWifi {

    class MyParallelSocketReactor {
    public:
        explicit MyParallelSocketReactor(uint32_t NumReactors=8) :
            NumReactors_(NumReactors)
        {
            Reactors_ = new Poco::Net::SocketReactor[NumReactors_];
            for(uint32_t i=0;i<NumReactors_;i++) {
                ReactorPool_.start(Reactors_[i]);
            }
        }

        ~MyParallelSocketReactor() {
            for(uint32_t i=0;i<NumReactors_;i++) {
                Reactors_[i].stop();
            }
            ReactorPool_.stopAll();
            ReactorPool_.joinAll();
            delete [] Reactors_;
        }

        Poco::Net::SocketReactor & Reactor() {
            return Reactors_[ rand() % NumReactors_ ];
        }

    private:
        uint32_t                   NumReactors_;
        Poco::Net::SocketReactor   * Reactors_;
        Poco::ThreadPool           ReactorPool_;
    };

    class WebSocketClient;

    class WebSocketClientServer : public SubSystemServer, Poco::Runnable {
        public:
            static auto instance() {
                static auto instance_ = new WebSocketClientServer;
                return instance_;
            }

            int Start() override;
            void Stop() override;
            void run() override;
            inline MyParallelSocketReactor & ReactorPool() { return *ReactorPool_; }
            inline bool Register( WebSocketClient * Client, const std::string &Id) {
                std::lock_guard G(Mutex_);
                Clients_[Id] = std::make_pair(Client,"");
                return true;
            }

            inline void UnRegister(const std::string &Id) {
                std::lock_guard G(Mutex_);
                Clients_.erase(Id);
            }

            inline void SetUser(const std::string &Id, const std::string &UserId) {
                std::lock_guard G(Mutex_);

                auto it=Clients_.find(Id);
                if(it!=Clients_.end()) {
                    Clients_[Id] = std::make_pair(it->second.first,UserId);
                }
            }

            inline bool GeoCodeEnabled() const { return GeoCodeEnabled_; }
            [[nodiscard]] inline std::string GoogleApiKey() const { return GoogleApiKey_; }
            [[nodiscard]] bool Send(const std::string &Id, const std::string &Payload);
            [[nodiscard]] bool SendUserNotification(const std::string &userName, const ProvObjects::WebSocketNotification & Notification);

    private:
            std::atomic_bool                            Running_=false;
            Poco::Thread                                Thr_;
            std::unique_ptr<MyParallelSocketReactor>    ReactorPool_;
            bool                                        GeoCodeEnabled_=false;
            std::string                                 GoogleApiKey_;
            std::map<std::string, std::pair<WebSocketClient *,std::string>>     Clients_;

            [[nodiscard]] bool SendToUser(const std::string &userName, const std::string &Payload);

            WebSocketClientServer() noexcept:
                SubSystemServer("WebSocketClientServer", "WSCLNT-SVR", "websocketclients")
                {
                }
        };

    inline auto WebSocketClientServer() { return WebSocketClientServer::instance(); }

    class WebSocketClient {
    public:
        explicit WebSocketClient( Poco::Net::WebSocket & WS , const std::string &Id, Poco::Logger & L) :
            Reactor_(WebSocketClientServer()->ReactorPool().Reactor()),
            Id_(Id),
            Logger_(L) {
            try {
                WS_ = std::make_unique<Poco::Net::WebSocket>(WS);
                Reactor_.addEventHandler(*WS_,
                                         Poco::NObserver<WebSocketClient, Poco::Net::ReadableNotification>(
                                                 *this, &WebSocketClient::OnSocketReadable));
                Reactor_.addEventHandler(*WS_,
                                         Poco::NObserver<WebSocketClient, Poco::Net::ShutdownNotification>(
                                                 *this, &WebSocketClient::OnSocketShutdown));
                Reactor_.addEventHandler(*WS_,
                                         Poco::NObserver<WebSocketClient, Poco::Net::ErrorNotification>(
                                                 *this, &WebSocketClient::OnSocketError));
                WebSocketClientServer()->Register(this, Id_);
            } catch (...) {
                delete this;
            }
        }

        ~WebSocketClient() {
            try {
                WebSocketClientServer()->UnRegister(Id_);
                Reactor_.removeEventHandler(*WS_,
                                            Poco::NObserver<WebSocketClient,
                                            Poco::Net::ReadableNotification>(*this,&WebSocketClient::OnSocketReadable));
                Reactor_.removeEventHandler(*WS_,
                                            Poco::NObserver<WebSocketClient,
                                            Poco::Net::ShutdownNotification>(*this,&WebSocketClient::OnSocketShutdown));
                Reactor_.removeEventHandler(*WS_,
                                            Poco::NObserver<WebSocketClient,
                                            Poco::Net::ErrorNotification>(*this,&WebSocketClient::OnSocketError));
                (*WS_).shutdown();
                (*WS_).close();
            } catch(...) {

            }
        }

        void ws_command_serial_number_search( const Poco::JSON::Object::Ptr &O, bool &Done, std::string &Answer);
        void ws_command_address_completion( const Poco::JSON::Object::Ptr &O, bool &Done, std::string &Answer);
        void ws_command_exit( const Poco::JSON::Object::Ptr &O, bool &Done, std::string &Answer);
        void ws_command_invalid( const Poco::JSON::Object::Ptr &O, bool &Done, std::string &Answer);
        void ws_command_subuser_search( const Poco::JSON::Object::Ptr &O, bool &Done, std::string &Answer);
        void ws_command_subdevice_search( const Poco::JSON::Object::Ptr &O, bool &Done, std::string &Answer);

        [[nodiscard]] inline const std::string & Id() { return Id_; };

        std::string GoogleGeoCodeCall(const std::string &A);
        [[nodiscard]] inline bool Send(const std::string &Payload) {
            try {
                WS_->sendFrame(Payload.c_str(),Payload.size());
                return true;
            } catch (...) {

            }
            return false;
        }
        Poco::Logger & Logger() { return Logger_;}
        private:
            std::unique_ptr<Poco::Net::WebSocket>     WS_;
            Poco::Net::SocketReactor                & Reactor_;
            std::string                               Id_;
            Poco::Logger                            & Logger_;
            bool                                      Authenticated_=false;
            SecurityObjects::UserInfoAndPolicy        UserInfo_;

            void Process(const Poco::JSON::Object::Ptr &O, std::string &Answer, bool &Done );
            void OnSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf);
            void OnSocketShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf);
            void OnSocketError(const Poco::AutoPtr<Poco::Net::ErrorNotification>& pNf);
        };


}

