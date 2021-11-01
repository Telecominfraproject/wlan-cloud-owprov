//
// Created by stephane bourque on 2021-11-01.
//

#include "WebSocketClientServer.h"
#include "SerialNumberCache.h"

namespace OpenWifi {
    void WebSocketClientServer::run() {
        Running_ = true ;
        while(Running_) {
            Poco::Thread::trySleep(2000);

            if(!Running_)
                break;
        }
    };

    int WebSocketClientServer::Start() {
        GoogleApiKey_ = MicroService::instance().ConfigGetString("google.apikey","");
        GeoCodeEnabled_ = !GoogleApiKey_.empty();
        ReactorPool_ = std::make_unique<MyParallelSocketReactor>();
        Thr_.start(*this);
        return 0;
    };

    void WebSocketClientServer::Stop() {
        if(Running_) {
            Running_ = false;
            Thr_.wakeUp();
            Thr_.join();
        }
    };

    void WebSocketClient::OnSocketError(const Poco::AutoPtr<Poco::Net::ErrorNotification> &pNf) {
        delete this;
    }

    bool WebSocketClientServer::Send(const std::string &Id, const std::string &Payload) {
        std::lock_guard G(Mutex_);

        auto It = Clients_.find(Id);
        if(It!=Clients_.end())
            return It->second->Send(Payload);
        return false;
    }

    void WebSocketClient::OnSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf) {
        int flags;
        int n;
        bool Done=false;
        Poco::Buffer<char>			IncomingFrame(0);
        n = WS_->receiveFrame(IncomingFrame, flags);
        auto Op = flags & Poco::Net::WebSocket::FRAME_OP_BITMASK;

        if(n==0) {
            return delete this;
        }

        switch(Op) {
            case Poco::Net::WebSocket::FRAME_OP_PING: {
                WS_->sendFrame("", 0,
                             (int)Poco::Net::WebSocket::FRAME_OP_PONG |
                             (int)Poco::Net::WebSocket::FRAME_FLAG_FIN);
                }
                break;
            case Poco::Net::WebSocket::FRAME_OP_PONG: {
                }
                break;
            case Poco::Net::WebSocket::FRAME_OP_CLOSE: {
                    Logger_.warning(Poco::format("CLOSE(%s): Client is closing its connection.",Id_));
                    Done=true;
                }
                break;
            case Poco::Net::WebSocket::FRAME_OP_TEXT: {
                IncomingFrame.append(0);
                if(!Authenticated_) {
                    std::string Frame{IncomingFrame.begin()};
                    auto Tokens = Utils::Split(Frame,':');
                    if(Tokens.size()==2 && AuthClient()->IsTokenAuthorized(Tokens[1], UserInfo_)) {
                        Authenticated_=true;
                        std::string S{"Welcome! Bienvenue! Bienvenidos!"};
                        WS_->sendFrame(S.c_str(),S.size());
                    } else {
                        std::string S{"Invalid token. Closing connection."};
                        WS_->sendFrame(S.c_str(),S.size());
                        Done=true;
                    }

                } else {
                    try {
                        Poco::JSON::Parser P;
                        auto Obj = P.parse(IncomingFrame.begin())
                                .extract<Poco::JSON::Object::Ptr>();
                        std::string Answer;
                        Process(Obj, Answer, Done );
                        if (!Answer.empty())
                            WS_->sendFrame(Answer.c_str(), Answer.size());
                        else {
                            WS_->sendFrame("{}", 2);
                        }
                    } catch (const Poco::JSON::JSONException & E) {
                        Logger_.log(E);
                    }
                }
            }
            break;
            default:
            {

            }
        }

        if(Done) {
            delete this;
        }
    }

    void WebSocketClient::OnSocketShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification> &pNf) {
        delete this;
    }

    void WebSocketClient::Process(const Poco::JSON::Object::Ptr &O, std::string &Answer, bool &Done ) {
        try {
            if (O->has("command")) {
                auto Command = O->get("command").toString();
                if (Command == "serial_number_search" && O->has("serial_prefix")) {
                    auto Prefix = O->get("serial_prefix").toString();
                    uint64_t HowMany = 32;
                    if (O->has("howMany"))
                        HowMany = O->get("howMany");
                    Logger_.information(Poco::format("serial_number_search: %s", Prefix));
                    if (!Prefix.empty() && Prefix.length() < 13) {
                        std::vector<uint64_t> Numbers;
                        SerialNumberCache()->FindNumbers(Prefix, 50, Numbers);
                        Poco::JSON::Array A;
                        for (const auto &i : Numbers)
                            A.add(Utils::int_to_hex(i));
                        Poco::JSON::Object AO;
                        AO.set("serialNumbers", A);
                        AO.set("command","serial_number_search");
                        std::ostringstream SS;
                        Poco::JSON::Stringifier::stringify(AO, SS);
                        Answer = SS.str();
                    }
                } else if (WebSocketClientServer()->GeoCodeEnabled() && Command == "address_completion" && O->has("address")) {
                    auto Address = O->get("address").toString();
                    Answer = GoogleGeoCodeCall(Address);
                } else if (Command=="exit") {
                    Answer = R"lit({ "closing" : "Goodbye! Aurevoir! Hasta la vista!" })lit";
                    Done = true;
                } else {
                    Answer = std::string{R"lit({ "error" : "invalid command" })lit"};
                }
            }
        } catch (const Poco::Exception &E) {
            Logger_.log(E);
        }
    }

    std::string WebSocketClient::GoogleGeoCodeCall(const std::string &A) {
        try {
            std::string URI = { "https://maps.googleapis.com/maps/api/geocode/json"};
            Poco::URI   uri(URI);

            uri.addQueryParameter("address",A);
            uri.addQueryParameter("key", WebSocketClientServer()->GoogleApiKey());

            Poco::Net::HTTPSClientSession session(uri.getHost(), uri.getPort());
            Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_GET, uri.getPathAndQuery(), Poco::Net::HTTPMessage::HTTP_1_1);
            session.sendRequest(req);
            Poco::Net::HTTPResponse res;
            std::istream& rs = session.receiveResponse(res);

            if(res.getStatus()==Poco::Net::HTTPResponse::HTTP_OK) {
                std::ostringstream os;
                Poco::StreamCopier::copyStream(rs,os);
                return os.str();
            } else {
                std::ostringstream os;
                Poco::StreamCopier::copyStream(rs,os);
                return R"lit({ "error: )lit" + os.str() + R"lit( })lit";
            }
        } catch(...) {

        }
        return "{ \"error\" : \"No call made\" }";
    }

}