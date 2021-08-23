//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//


#include "RESTAPI_venue_handler.h"

namespace OpenWifi{
    void RESTAPI_venue_handler::handleRequest(Poco::Net::HTTPServerRequest &Request,
                                                Poco::Net::HTTPServerResponse &Response) {
        if (!ContinueProcessing(Request, Response))
            return;

        if (!IsAuthorized(Request, Response))
            return;

        ParseParameters(Request);
        if(Request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET)
            DoGet(Request, Response);
        else if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST)
            DoPost(Request, Response);
        else if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_DELETE)
            DoDelete(Request, Response);
        else if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_PUT)
            DoPut(Request, Response);
        else
            BadRequest(Request, Response, "Unknown HTTP Method");
    }

    void RESTAPI_venue_handler::DoGet(Poco::Net::HTTPServerRequest &Request,
                                        Poco::Net::HTTPServerResponse &Response) {

    }

    void RESTAPI_venue_handler::DoDelete(Poco::Net::HTTPServerRequest &Request,
                                           Poco::Net::HTTPServerResponse &Response) {

    }

    void RESTAPI_venue_handler::DoPost(Poco::Net::HTTPServerRequest &Request,
                                        Poco::Net::HTTPServerResponse &Response) {

    }

    void RESTAPI_venue_handler::DoPut(Poco::Net::HTTPServerRequest &Request,
                                        Poco::Net::HTTPServerResponse &Response) {

    }
}