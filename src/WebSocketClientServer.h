//
// Created by stephane bourque on 2021-11-01.
//

#ifndef OWPROV_WEBSOCKETCLIENTSERVER_H
#define OWPROV_WEBSOCKETCLIENTSERVER_H

#include "framework/MicroService.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Environment.h"
#include "Poco/NObserver.h"
#include "Poco/Net/SocketNotification.h"

#include "RESTObjects/RESTAPI_SecurityObjects.h"

namespace OpenWifi {

    class MyParallelSocketReactor {
    public:
        MyParallelSocketReactor() {
            for(int i=0;i<NumReactors_;i++) {
                Threads_[i].start(Reactors_[i]);
            }
        }

        ~MyParallelSocketReactor() {
            for(int i=0;i<NumReactors_;i++) {
                Reactors_[i].stop();
                Threads_[i].join();
            }
        }

        Poco::Net::SocketReactor & Reactor() {
            return Reactors_[ rand() % NumReactors_ ];
        }

    private:
        unsigned    NumReactors_=8;
        std::array<Poco::Net::SocketReactor,8> Reactors_;
        std::array<Poco::Thread,8>             Threads_;
    };

    class WebSocketClient;

    class WebSocketClientServer : public SubSystemServer, Poco::Runnable {
        public:
            static WebSocketClientServer *instance() {
                static WebSocketClientServer * instance_ = new WebSocketClientServer;
                return instance_;
            }

            int Start() override;
            void Stop() override;
            void run() override;
            inline MyParallelSocketReactor & ReactorPool() { return *ReactorPool_; }
            inline bool Register( WebSocketClient * Client, const std::string &Id) {
                std::lock_guard G(Mutex_);
                Clients_[Id] = Client;
                return true;
            }

            inline void UnRegister(const std::string &Id) {
                std::lock_guard G(Mutex_);
                Clients_.erase(Id);

            }

            inline bool GeoCodeEnabled() const { return GeoCodeEnabled_; }
            [[nodiscard]] inline std::string GoogleApiKey() const { return GoogleApiKey_; }
            [[nodiscard]] bool Send(const std::string &Id, const std::string &Payload);

        private:
            std::atomic_bool                            Running_=false;
            Poco::Thread                                Thr_;
            std::unique_ptr<MyParallelSocketReactor>    ReactorPool_;
            bool                                        GeoCodeEnabled_=false;
            std::string                                 GoogleApiKey_;
            std::map<std::string,WebSocketClient *>     Clients_;

            WebSocketClientServer() noexcept:
                SubSystemServer("WebSocketClientServer", "WSCLNT-SVR", "websocketclients")
                {
                }
        };

    inline WebSocketClientServer * WebSocketClientServer() { return WebSocketClientServer::instance(); }

    class WebSocketClient {
    public:
        explicit WebSocketClient( Poco::Net::WebSocket & WS , const std::string Id, Poco::Logger & L) :
            Reactor_(WebSocketClientServer()->ReactorPool().Reactor()),
            Id_(std::move(Id)),
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

#endif //OWPROV_WEBSOCKETCLIENTSERVER_H
