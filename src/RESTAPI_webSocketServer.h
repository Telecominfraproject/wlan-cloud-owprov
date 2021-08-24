//
// Created by stephane bourque on 2021-08-12.
//

#ifndef UCENTRALGW_RESTAPI_WEBSOCKETSERVER_H
#define UCENTRALGW_RESTAPI_WEBSOCKETSERVER_H

#include <functional>

#include "RESTAPI_handler.h"

namespace OpenWifi {

    typedef std::function<void(const Poco::JSON::Object::Ptr &, std::string &, const SecurityObjects::UserInfoAndPolicy &)> ws_processor_func;

	class RESTAPI_webSocketServer : public RESTAPIHandler {
	  public:
		RESTAPI_webSocketServer(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, bool Internal)
		: RESTAPIHandler(bindings, L,
						 std::vector<std::string>{Poco::Net::HTTPRequest::HTTP_GET,
												  Poco::Net::HTTPRequest::HTTP_OPTIONS},
												  Internal) {}
		void handleRequest(Poco::Net::HTTPServerRequest &Request,
						 Poco::Net::HTTPServerResponse &Response) override final;
		static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/ws"};}
		void DoGet(Poco::Net::HTTPServerRequest &Request,
				   Poco::Net::HTTPServerResponse &Response);

		inline void RegisterProcessor(const std::string &Command, ws_processor_func & f) {
		    CommandProcessors_[Command] = f;
		}

	  private:
		void Process(const Poco::JSON::Object::Ptr &O, std::string &Answer);
		std::map<std::string,ws_processor_func>     CommandProcessors_;
	};
}

#endif // UCENTRALGW_RESTAPI_WEBSOCKETSERVER_H
