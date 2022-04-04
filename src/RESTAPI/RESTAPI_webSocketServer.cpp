//
// Created by stephane bourque on 2021-08-12.
//

#include "RESTAPI_webSocketServer.h"

#include "framework/MicroService.h"

#include "Poco/Net/WebSocket.h"
#include "Poco/Net/NetException.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Stringifier.h"
#include "Poco/Net/HTTPSClientSession.h"

#include "SerialNumberCache.h"
#include "WebSocketClientServer.h"

namespace OpenWifi {

    void RESTAPI_webSocketServer::DoGet() {
        try
        {
            if(Request->find("Upgrade") != Request->end() && Poco::icompare((*Request)["Upgrade"], "websocket") == 0) {
                try
                {
                    Poco::Net::WebSocket WS(*Request, *Response);
                    Logger().information("WebSocket connection established.");
                    auto Id = MicroService::CreateUUID();
                    new WebSocketClient(WS,Id,Logger());
                }
                catch (...) {
                    std::cout << "Cannot create websocket client..." << std::endl;
                }
            }
        } catch(...) {
            std::cout << "Cannot upgrade connection..." << std::endl;
        }
    }

	void RESTAPI_webSocketServer::Process(const Poco::JSON::Object::Ptr &O, std::string &Answer, bool &Done ) {
	    try {
	        if (O->has("command")) {
	            auto Command = O->get("command").toString();
	            if (Command == "serial_number_search" && O->has("serial_prefix")) {
	                auto Prefix = O->get("serial_prefix").toString();
	                Logger().information(Poco::format("serial_number_search: %s", Prefix));
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
	            } else if (GeoCodeEnabled_ && Command == "address_completion" && O->has("address")) {
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
			Logger().log(E);
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