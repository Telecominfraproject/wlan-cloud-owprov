//
// Created by stephane bourque on 2021-08-12.
//

#include "RESTAPI_webSocketServer.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Stringifier.h"

#include "framework/Utils.h"
#include "framework/AuthClient.h"
#include "framework/RESTAPI_errors.h"

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
										Process(Obj, Answer);
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
		} else {
		    BadRequest(RESTAPI::Errors::OnlyWSSupported);
		}
	}

	void RESTAPI_webSocketServer::Process(const Poco::JSON::Object::Ptr &O, std::string &Answer ) {
		try {
			if (O->has("command")) {
				auto Command = O->get("command").toString();
				auto It = CommandProcessors_.find(Command);
				if(It!=CommandProcessors_.end())
				    It->second(O,Answer,UserInfo_);
			}
		} catch (const Poco::Exception &E) {
			Logger_.log(E);
		}
	}
}