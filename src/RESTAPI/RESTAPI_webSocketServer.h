//
// Created by stephane bourque on 2021-08-12.
//

#ifndef UCENTRALGW_RESTAPI_WEBSOCKETSERVER_H
#define UCENTRALGW_RESTAPI_WEBSOCKETSERVER_H

#include <functional>

#include "framework/RESTAPI_handler.h"

namespace OpenWifi {

    typedef std::function<void(const Poco::JSON::Object::Ptr &, std::string &, const SecurityObjects::UserInfoAndPolicy &)> ws_processor_func;

	class RESTAPI_webSocketServer : public RESTAPIHandler {
	  public:
	    RESTAPI_webSocketServer(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer & Server, bool Internal)
		: RESTAPIHandler(bindings, L,
						 std::vector<std::string>{Poco::Net::HTTPRequest::HTTP_GET,
												  Poco::Net::HTTPRequest::HTTP_OPTIONS},
												  Server,
												  Internal) {}
		static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/ws"};}

		void DoGet() final;
		void DoPost() final {};
		void DoPut() final {};
		void DoDelete() final {};

		inline void RegisterProcessor(const std::string &Command, ws_processor_func & f) {
		    CommandProcessors_[Command] = f;
		}

	  private:
		void Process(const Poco::JSON::Object::Ptr &O, std::string &Answer);
		std::map<std::string,ws_processor_func>     CommandProcessors_;
	};
}

#endif // UCENTRALGW_RESTAPI_WEBSOCKETSERVER_H
