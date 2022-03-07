//
// Created by stephane bourque on 2022-01-11.
//

#include "SDK_sec.h"

namespace OpenWifi::SDK::Sec {

    namespace User {
        bool Exists(RESTAPIHandler *client, const Types::UUID_t & Id) {
            OpenAPIRequestGet	Req(    uSERVICE_SECURITY,
                                         "/api/v1/user/" + Id,
                                         {},
                                         5000);

            auto CallResponse = Poco::makeShared<Poco::JSON::Object>();
            auto StatusCode = Req.Do(CallResponse);
            if( StatusCode == Poco::Net::HTTPResponse::HTTP_OK) {
                return true;
            }
            return false;
        }

        bool Get(RESTAPIHandler *client, const Types::UUID_t & Id, SecurityObjects::UserInfo & UserInfo) {
            OpenAPIRequestGet	Req(    uSERVICE_SECURITY,
                                         "/api/v1/user/" + Id,
                                         {},
                                         5000);

            auto CallResponse = Poco::makeShared<Poco::JSON::Object>();
            auto StatusCode = Req.Do(CallResponse);
            if( StatusCode == Poco::Net::HTTPResponse::HTTP_OK) {
                return UserInfo.from_json(CallResponse);
            }
            return false;
        }
    }

    namespace Subscriber {
        bool Exists(RESTAPIHandler *client, const Types::UUID_t & Id) {
            OpenAPIRequestGet	Req(    uSERVICE_SECURITY,
                                         "/api/v1/subuser/" + Id,
                                         {},
                                         5000);

            auto CallResponse = Poco::makeShared<Poco::JSON::Object>();
            auto StatusCode = Req.Do(CallResponse);
            if( StatusCode == Poco::Net::HTTPResponse::HTTP_OK) {
                return true;
            }
            return false;
        }

        bool Get(RESTAPIHandler *client, const Types::UUID_t & Id, SecurityObjects::UserInfo & UserInfo) {
            OpenAPIRequestGet	Req(    uSERVICE_SECURITY,
                                         "/api/v1/subuser/" + Id,
                                         {},
                                         5000);

            auto CallResponse = Poco::makeShared<Poco::JSON::Object>();
            auto StatusCode = Req.Do(CallResponse);
            if( StatusCode == Poco::Net::HTTPResponse::HTTP_OK) {
                return UserInfo.from_json(CallResponse);
            }
            return false;
        }

        bool Delete(RESTAPIHandler *client, const Types::UUID_t & Id) {
            OpenAPIRequestDelete	Req(    uSERVICE_SECURITY,
                                         "/api/v1/subuser/" + Id,
                                         {},
                                         5000);

            auto StatusCode = Req.Do();
            if( StatusCode == Poco::Net::HTTPResponse::HTTP_OK ||
                StatusCode == Poco::Net::HTTPResponse::HTTP_NO_CONTENT ||
                StatusCode == Poco::Net::HTTPResponse::HTTP_ACCEPTED) {
                return true;
            }
            return false;
        }
    }

}
