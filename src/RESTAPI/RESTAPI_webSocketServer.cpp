//
// Created by stephane bourque on 2021-08-12.
//

#include "RESTAPI_webSocketServer.h"

#include "framework/MicroService.h"

#include "Poco/Net/WebSocket.h"
#include "Poco/Net/NetException.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Stringifier.h"
#include "Poco/Net/HTTPSClientSession.h"

#include "SerialNumberCache.h"

namespace OpenWifi {

	void RESTAPI_webSocketServer::DoGet() {

		//	try and upgrade this session to websocket...
		if(Request->find("Upgrade") != Request->end() && Poco::icompare((*Request)["Upgrade"], "websocket") == 0) {
			try
			{
				Poco::Net::WebSocket WS(*Request, *Response);
				Logger_.information("WebSocket connection established.");
				int flags;
				int n;
				bool Authenticated=false;
				bool Done=false;
				GoogleApiKey_ = MicroService::instance().ConfigGetString("google.apikey","");
				GeoCodeEnabled_ = !GoogleApiKey_.empty();

				do
				{
					Poco::Buffer<char>			IncomingFrame(0);
					n = WS.receiveFrame(IncomingFrame, flags);
					auto Op = flags & Poco::Net::WebSocket::FRAME_OP_BITMASK;
					switch(Op) {
						case Poco::Net::WebSocket::FRAME_OP_PING: {
							WS.sendFrame("", 0,
										   (int)Poco::Net::WebSocket::FRAME_OP_PONG |
										   (int)Poco::Net::WebSocket::FRAME_FLAG_FIN);
							}
							break;
						case Poco::Net::WebSocket::FRAME_OP_PONG: {
							}
							break;
						case Poco::Net::WebSocket::FRAME_OP_TEXT: {
								IncomingFrame.append(0);
								if(!Authenticated) {
									std::string Frame{IncomingFrame.begin()};
									auto Tokens = Utils::Split(Frame,':');
									if(Tokens.size()==2 && AuthClient()->IsTokenAuthorized(Tokens[1], UserInfo_)) {
										Authenticated=true;
										std::string S{"Welcome! Bienvenue! Bienvenidos!"};
										WS.sendFrame(S.c_str(),S.size());
									} else {
										std::string S{"Invalid token. Closing connection."};
										WS.sendFrame(S.c_str(),S.size());
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
											WS.sendFrame(Answer.c_str(), Answer.size());
										else {
											WS.sendFrame("{}", 2);
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
				}
				while (!Done && (n > 0 && (flags & Poco::Net::WebSocket::FRAME_OP_BITMASK) != Poco::Net::WebSocket::FRAME_OP_CLOSE));
				Logger_.information("WebSocket connection closed.");
			}
			catch (const Poco::Net::WebSocketException & E)
			{
				Logger_.log(E);
				switch (E.code())
				{
				case Poco::Net::WebSocket::WS_ERR_HANDSHAKE_UNSUPPORTED_VERSION:
					Response->set("Sec-WebSocket-Version", Poco::Net::WebSocket::WEBSOCKET_VERSION);
					// fallthrough
					case Poco::Net::WebSocket::WS_ERR_NO_HANDSHAKE:
						case Poco::Net::WebSocket::WS_ERR_HANDSHAKE_NO_VERSION:
							case Poco::Net::WebSocket::WS_ERR_HANDSHAKE_NO_KEY:
								Response->setStatusAndReason(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
								Response->setContentLength(0);
								Response->send();
								break;
				}
			}
			catch (const Poco::Exception &E) {
				Logger_.log(E);
			}
		}
	}

	void RESTAPI_webSocketServer::Process(const Poco::JSON::Object::Ptr &O, std::string &Answer, bool &Done ) {
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
	            } else if(GeoCodeEnabled_ && Command == "address_completion" && O->has("address")) {
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

    std::string RESTAPI_webSocketServer::GoogleGeoCodeCall(const std::string &A) {
	    try {
	        std::string URI = { "https://maps.googleapis.com/maps/api/geocode/json"};
	        Poco::URI   uri(URI);

	        uri.addQueryParameter("address",A);
	        uri.addQueryParameter("key", GoogleApiKey_);

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