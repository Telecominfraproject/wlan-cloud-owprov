//
// Created by stephane bourque on 2021-11-01.
//

#include "framework/MicroService.h"
#include "WebSocketClientServer.h"
#include "SerialNumberCache.h"
#include "StorageService.h"
#include "sdks/SDK_sec.h"

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

    bool WebSocketClientServer::SendUserNotification(const std::string &userName,
                                                     const ProvObjects::WebSocketNotification &Notification) {

        Poco::JSON::Object  Payload;
        Notification.to_json(Payload);
        std::ostringstream OO;
        Payload.stringify(OO);
        return SendToUser(userName,OO.str());
    }

    void WebSocketClient::OnSocketError([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ErrorNotification> &pNf) {
        delete this;
    }

    bool WebSocketClientServer::Send(const std::string &Id, const std::string &Payload) {
        std::lock_guard G(Mutex_);

        auto It = Clients_.find(Id);
        if(It!=Clients_.end())
            return It->second.first->Send(Payload);
        return false;
    }

    bool WebSocketClientServer::SendToUser(const std::string &UserName, const std::string &Payload) {
        std::lock_guard G(Mutex_);
        uint64_t Sent=0;

        for(const auto &client:Clients_) {
            if(client.second.second == UserName) {
                 if(client.second.first->Send(Payload))
                     Sent++;
            }
        }
        return Sent>0;
    }

    void WebSocketClient::OnSocketReadable([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf) {
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
                    Logger().warning(Poco::format("CLOSE(%s): Client is closing its connection.",Id_));
                    Done=true;
                }
                break;
            case Poco::Net::WebSocket::FRAME_OP_TEXT: {
                IncomingFrame.append(0);
                if(!Authenticated_) {
                    std::string Frame{IncomingFrame.begin()};
                    auto Tokens = Utils::Split(Frame,':');
                    bool Expired = false, Contacted = false;
                    if(Tokens.size()==2 && AuthClient()->IsAuthorized(Tokens[1], UserInfo_, Expired, Contacted)) {
                        Authenticated_=true;
                        std::string S{"Welcome! Bienvenue! Bienvenidos!"};
                        WS_->sendFrame(S.c_str(),S.size());
                        WebSocketClientServer()->SetUser(Id_,UserInfo_.userinfo.email);
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
                            WS_->sendFrame(Answer.c_str(), (int) Answer.size());
                        else {
                            WS_->sendFrame("{}", 2);
                        }
                    } catch (const Poco::JSON::JSONException & E) {
                        Logger().log(E);
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

    void WebSocketClient::OnSocketShutdown([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ShutdownNotification> &pNf) {
        delete this;
    }

    void WebSocketClient::ws_command_serial_number_search(const Poco::JSON::Object::Ptr &O,
                                                          bool &Done, std::string &Answer) {
        Done = false;
        auto Prefix = O->get("serial_prefix").toString();
        Logger().information(Poco::format("serial_number_search: %s", Prefix));
        if (!Prefix.empty() && Prefix.length() < 13) {
            std::vector<uint64_t> Numbers;
            SerialNumberCache()->FindNumbers(Prefix, 50, Numbers);
            Poco::JSON::Array A;
            for (const auto &i : Numbers)
                A.add(Utils::int_to_hex(i));
            Poco::JSON::Object A0;
            A0.set("serialNumbers", A0);
            std::ostringstream SS;
            Poco::JSON::Stringifier::stringify(A, SS);
            Answer = SS.str();
        }
    }

    void
    WebSocketClient::ws_command_address_completion(const Poco::JSON::Object::Ptr &O, bool &Done, std::string &Answer) {
        Done = false;
        auto Address = O->get("address").toString();
        Answer = GoogleGeoCodeCall(Address);
    }

    void WebSocketClient::ws_command_exit([[maybe_unused]] const Poco::JSON::Object::Ptr &O, bool &Done, std::string &Answer) {
        Done = true;
        Answer = R"lit({ "closing" : "Goodbye! Aurevoir! Hasta la vista!" })lit";
    }

    void WebSocketClient::ws_command_invalid([[maybe_unused]] const Poco::JSON::Object::Ptr &O, bool &Done, std::string &Answer) {
        Done = false;
        Answer = std::string{R"lit({ "error" : "invalid command" })lit"};
    }

    void WebSocketClient::ws_command_subuser_search( const Poco::JSON::Object::Ptr &O, bool &Done, std::string &Answer) {
        Done = false;
        auto operatorId = O->get("operatorId").toString();
        std::string nameSearch, emailSearch;
        OpenWifi::RESTAPIHandler::AssignIfPresent(O,"nameSearch",nameSearch);
        OpenWifi::RESTAPIHandler::AssignIfPresent(O,"emailSearch",emailSearch);
        SecurityObjects::UserInfoList   Users;
        SDK::Sec::Subscriber::Search(nullptr,operatorId,nameSearch,emailSearch,Users);

        Poco::JSON::Array   Arr;
        for(const auto &i:Users.users) {
            Poco::JSON::Object  OO;
            OO.set("name", i.name);
            OO.set("email", i.email);
            OO.set("id", i.id);
            i.to_json(OO);
            Arr.add(OO);
        }
        Poco::JSON::Object  ObjAnswer;
        ObjAnswer.set("users", Arr);
        std::ostringstream SS;
        Poco::JSON::Stringifier::stringify(ObjAnswer, SS);
        Answer = SS.str();
    }

    void WebSocketClient::ws_command_subdevice_search( const Poco::JSON::Object::Ptr &O, bool &Done, std::string &Answer) {
        Done = false;
        auto operatorId = O->get("operatorId").toString();
        auto Prefix = O->get("serial_prefix").toString();
        std::string Query;

        if(Prefix[0]=='*') {
            Query = fmt::format(" operatorId='{}' and (right(serialNumber,{})='{}' or right(realMacAddress,{})='{}' ) ",
                                operatorId, Prefix.size()-1, Prefix.substr(1), Prefix.size()-1, Prefix.substr(1));
        } else {
            Query = fmt::format(" operatorId='{}' and (left(serialNumber,{})='{}'  or left(realMacAddress,{})='{}' ) ",
                                operatorId, Prefix.size(), Prefix, Prefix.size(), Prefix);
        }

        std::vector<ProvObjects::SubscriberDevice>  SubDevices;

        StorageService()->SubscriberDeviceDB().GetRecords(0,200,SubDevices,Query);
        Poco::JSON::Array   Arr;
        for(const auto &i:SubDevices) {
            Arr.add(i.serialNumber);
        }
        Poco::JSON::Object  RetObj;
        RetObj.set("serialNumbers", Arr);
        std::ostringstream SS;
        Poco::JSON::Stringifier::stringify(RetObj, SS);
        Answer = SS.str();
    }

    void WebSocketClient::Process(const Poco::JSON::Object::Ptr &O, std::string &Result, bool &Done ) {
        try {
            if (O->has("command") && O->has("id")) {
                auto id = (uint64_t) O->get("id");
                std::string Answer;
                auto Command = O->get("command").toString();
                if (Command == "serial_number_search" && O->has("serial_prefix")) {
                    ws_command_serial_number_search(O,Done,Answer);
                } else if (WebSocketClientServer()->GeoCodeEnabled() && Command == "address_completion" && O->has("address")) {
                    ws_command_address_completion(O,Done,Answer);
                } else if (WebSocketClientServer()->GeoCodeEnabled() && Command == "subuser_search" && O->has("operatorId")) {
                    ws_command_subuser_search(O,Done,Answer);
                } else if (WebSocketClientServer()->GeoCodeEnabled() && Command == "subdevice_search" && O->has("operatorId") && O->has("serial_prefix")) {
                    ws_command_subdevice_search(O,Done,Answer);
                } else if (Command=="exit") {
                    ws_command_exit(O,Done,Answer);
                } else {
                    ws_command_invalid(O,Done,Answer);
                }

                Result = fmt::format("{{ \"command_response_id\" : {} , \"response\" : {}  }}" , id, Answer);
            }
        } catch (const Poco::Exception &E) {
            Logger().log(E);
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