//
// Created by stephane bourque on 2022-01-11.
//

#include "framework/MicroService.h"
#include "SDK_gw.h"


namespace OpenWifi::SDK::GW {
    namespace Device {
        void Reboot(RESTAPIHandler *client, const std::string & Mac, uint64_t When) {
            std::string         EndPoint = "/api/v1/device/" + Mac + "/reboot";
            Poco::JSON::Object  ObjRequest;

            ObjRequest.set("serialNumber", Mac);
            ObjRequest.set("when",0);
            PerformCommand(client,"reboot",EndPoint, ObjRequest);
        }

        void LEDs(RESTAPIHandler *client, const std::string & Mac, uint64_t When, uint64_t Duration, const std::string & Pattern) {
            std::string         EndPoint = "/api/v1/device/" + Mac + "/leds";
            Poco::JSON::Object  ObjRequest;

            std::cout << "M:" << Mac << " P:" << Pattern << std::endl;

            ObjRequest.set("serialNumber", Mac);
            ObjRequest.set("when",When);
            ObjRequest.set("duration",Duration);
            ObjRequest.set("pattern", Pattern);
            PerformCommand(client,"leds",EndPoint, ObjRequest);
        }

        void Factory(RESTAPIHandler *client, const std::string & Mac, uint64_t When, bool KeepRedirector) {
            std::string         EndPoint = "/api/v1/device/" + Mac + "/factory";
            Poco::JSON::Object  ObjRequest;

            ObjRequest.set("serialNumber", Mac);
            ObjRequest.set("when",When);
            ObjRequest.set("keepRedirector",KeepRedirector);
            PerformCommand(client,"factory",EndPoint, ObjRequest);
        }

        void Upgrade(RESTAPIHandler *client, const std::string & Mac, uint64_t When, const std::string & ImageName, bool KeepRedirector) {
            std::string         EndPoint = "/api/v1/device/" + Mac + "/upgrade";
            Poco::JSON::Object  ObjRequest;

            ObjRequest.set("serialNumber", Mac);
            ObjRequest.set("when",When);
            ObjRequest.set("uri",ImageName);
            PerformCommand(client,"upgrade",EndPoint, ObjRequest);
        }

        void Refresh(RESTAPIHandler *client, const std::string & Mac, uint64_t When) {
            std::string         EndPoint = "/api/v1/device/" + Mac + "/refresh";
            Poco::JSON::Object  ObjRequest;

            ObjRequest.set("serialNumber", Mac);
            ObjRequest.set("when",When);
            PerformCommand(client,"refresh",EndPoint, ObjRequest);
        }

        void PerformCommand(RESTAPIHandler *client, const std::string &Command, const std::string & EndPoint, Poco::JSON::Object & CommandRequest) {
            auto API = OpenAPIRequestPost(uSERVICE_GATEWAY, EndPoint, {}, CommandRequest, 60000);
            Poco::JSON::Object::Ptr CallResponse;

            auto ResponseStatus = API.Do(CallResponse, client->UserInfo_.webtoken.access_token_);
            if (ResponseStatus == Poco::Net::HTTPServerResponse::HTTP_GATEWAY_TIMEOUT) {
                Poco::JSON::Object ResponseObject;
                ResponseObject.set("Code", Poco::Net::HTTPServerResponse::HTTP_GATEWAY_TIMEOUT);
                ResponseObject.set("Details",
                                   "Command could not complete, you may want to retry this operation later.");
                ResponseObject.set("Operation", Command);
                client->Response->setStatus(ResponseStatus);
                std::stringstream SS;
                Poco::JSON::Stringifier::condense(ResponseObject, SS);
                client->Response->setContentLength(SS.str().size());
                client->Response->setContentType("application/json");
                auto &os = client->Response->send();
                os << SS.str();
            } else {
                client->Response->setStatus(ResponseStatus);
                std::stringstream SS;
                Poco::JSON::Stringifier::condense(CallResponse, SS);
                Poco::JSON::Parser P;
                auto Raw = P.parse(SS.str()).extract<Poco::JSON::Object::Ptr>();
                if (Raw->has("command") && Raw->has("errorCode") && Raw->has("errorText")) {
                    Poco::JSON::Object ReturnResponse;
                    ReturnResponse.set("Operation", Raw->get("command").toString());
                    ReturnResponse.set("Details", Raw->get("errorText").toString());
                    ReturnResponse.set("Code", Raw->get("errorCode"));

                    std::stringstream Ret;
                    Poco::JSON::Stringifier::condense(ReturnResponse, Ret);
                    client->Response->setContentLength(Ret.str().size());
                    client->Response->setContentType("application/json");
                    auto &os = client->Response->send();
                    os << Ret.str();
                }
            }
        }

        bool SetVenue(RESTAPIHandler *client, const std::string & SerialNumber, const std::string &uuid) {
            Poco::JSON::Object      Body;

            Body.set("serialNumber", SerialNumber);
            Body.set("venue", uuid);
            OpenWifi::OpenAPIRequestPut R(OpenWifi::uSERVICE_GATEWAY,
                                          "/api/v1/device/" +SerialNumber,
                                          {},
                                          Body,
                                          10000);
            Poco::JSON::Object::Ptr Response;
            auto ResponseStatus = R.Do(Response, client ? client->UserInfo_.webtoken.access_token_ : "");
            if(ResponseStatus == Poco::Net::HTTPResponse::HTTP_OK) {
                return true;
            }
            return false;
        }

        bool Configure(RESTAPIHandler *client, const std::string &Mac, Poco::JSON::Object::Ptr & Configuration, Poco::JSON::Object::Ptr & Response) {

            Poco::JSON::Object      Body;

            Poco::JSON::Parser P;
            uint64_t Now = std::time(nullptr);

            Configuration->set("uuid", Now);
            Body.set("serialNumber", Mac);
            Body.set("UUID", Now);
            Body.set("when",0);
            Body.set("configuration", Configuration);

            OpenWifi::OpenAPIRequestPost R(OpenWifi::uSERVICE_GATEWAY,
                                           "/api/v1/device/" + Mac + "/configure",
                                           {},
                                           Body,
                                           10000);

            auto ResponseStatus = R.Do(Response, client ? client->UserInfo_.webtoken.access_token_ : "");
            if(ResponseStatus == Poco::Net::HTTPResponse::HTTP_OK) {
                std::ostringstream os;
                Poco::JSON::Stringifier::stringify(Response,os);
                return true;
            }

            return false;
        }

    }
}
