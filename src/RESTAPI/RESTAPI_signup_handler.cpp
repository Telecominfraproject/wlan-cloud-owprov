//
// Created by stephane bourque on 2022-02-20.
//

#include "RESTAPI_signup_handler.h"
#include "StorageService.h"
#include "Signup.h"

namespace OpenWifi {

    void RESTAPI_signup_handler::DoPost() {

        auto UserName = GetParameter("email","");
        Poco::toLowerInPlace(UserName);
        auto SerialNumber = GetParameter("serialNumber","");
        Poco::toLowerInPlace(SerialNumber);

        if(UserName.empty() || SerialNumber.empty()) {
            return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
        }

        if(!Utils::ValidEMailAddress(UserName)) {
            return BadRequest(RESTAPI::Errors::InvalidEmailAddress);
        }

        if(!Utils::ValidSerialNumber(SerialNumber)) {
            return BadRequest(RESTAPI::Errors::InvalidSerialNumber);
        }

        //  if a signup already exists for this user, we should just return its value
        //  or its completion
        SignupDB::RecordVec SEs;
        if(StorageService()->SignupDB().GetRecords(0,100, SEs, "email='" + UserName + "'")) {
            for(const auto &i:SEs) {
                if((i.created + Signup()->GracePeriod()) > OpenWifi::Now() && i.serialNumber==SerialNumber) {
                    Poco::JSON::Object  Answer;
                    i.to_json(Answer);
                    return ReturnObject(Answer);
                } else if((i.created + Signup()->GracePeriod()) < OpenWifi::Now() && i.completed==0) {
                    StorageService()->SignupDB().DeleteRecord("id", i.info.id);
                }
            }
        }

        //  So we do not have an outstanding signup...
        //  Can we actually claim this serial number??? if not, we need to return an error
        ProvObjects::InventoryTag   IT;
        if(StorageService()->InventoryDB().GetRecord("serialNumber",SerialNumber,IT)) {

            if(!IT.subscriber.empty()) {
                return BadRequest(RESTAPI::Errors::SerialNumberAlreadyProvisioned);
            }

            if(!(IT.devClass.empty() || IT.devClass=="subscriber" || IT.devClass=="any")) {
                return BadRequest(RESTAPI::Errors::SerialNumberNotTheProperClass);
            }
        }

        //  OK, we can claim this device, can we create a userid?
        //  Let's create one
        //  If sec.signup("email",uuid);
        auto SignupUUID = MicroService::instance().CreateUUID();

        Poco::JSON::Object  Body;
        OpenAPIRequestPost  CreateUser( uSERVICE_SECURITY, "/api/v1/signup", {
                { "email", UserName },
                { "signupUUID" , SignupUUID }
        }, Body, 30000);

        Poco::JSON::Object::Ptr Answer;
        if(CreateUser.Do(Answer)) {
            SecurityObjects::UserInfo   UI;

            UI.from_json(Answer);
            std::ostringstream os;
            Answer->stringify(os);
            std::cout << "Create user: " << std::endl << os.str() << std::endl;

            //  so create the Signup entry and modify the inventory
            ProvObjects::SignupEntry    SE;

            SE.info.id = SignupUUID;
            SE.info.created = SE.info.modified = SE.created = OpenWifi::Now();
            SE.completed = 0 ;
            SE.serialNumber = SerialNumber;
            SE.error = 0 ;
            SE.userId = UI.id;
            SE.email = UserName;
            SE.status = "waiting-for-email-verification";

            std::cout << "Creating signup entry: " << SE.email << std::endl;
            StorageService()->SignupDB().CreateRecord(SE);

            //  We do not have a device, so let's create one.
            if(IT.serialNumber.empty()) {
                IT.serialNumber = SerialNumber;
                IT.info.id = MicroService::instance().CreateUUID();
                IT.info.created = IT.info.modified = OpenWifi::Now();
                Poco::JSON::Object StateDoc;
                StateDoc.set("method", "signup");
                StateDoc.set("claimer", UserName);
                StateDoc.set("claimId", UI.id);
                StateDoc.set("errorCode",0);
                StateDoc.set("date", OpenWifi::Now());
                StateDoc.set("status", "waiting for email-verification");
                std::ostringstream os;
                StateDoc.stringify(os);
                IT.state = os.str();
                std::cout << "Creating inventory entry: " << SE.serialNumber << std::endl;
                StorageService()->InventoryDB().CreateRecord(IT);
            } else {
                Poco::JSON::Object StateDoc;
                StateDoc.set("method", "signup");
                StateDoc.set("claimer", UserName);
                StateDoc.set("claimId", UI.id);
                StateDoc.set("errorCode",0);
                StateDoc.set("date", OpenWifi::Now());
                StateDoc.set("status", "waiting for email-verification");
                std::ostringstream os;
                StateDoc.stringify(os);
                IT.state = os.str();
                IT.info.modified = OpenWifi::Now();
                std::cout << "Updating inventory entry: " << SE.serialNumber << std::endl;
                StorageService()->InventoryDB().UpdateRecord("id",IT.info.id,IT);
            }

            Poco::JSON::Object  SEAnswer;
            SE.to_json(SEAnswer);
            return ReturnObject(SEAnswer);
        }

        return BadRequest(RESTAPI::Errors::UserAlreadyExists);
    }

    //  this will be called by the SEC backend once the password has been verified.
    void RESTAPI_signup_handler::DoPut() {
        auto SignupUUID = GetParameter("signupUUID","");
        auto Operation = GetParameter("operation","");

        if(SignupUUID.empty() || Operation.empty()) {
            return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
        }

        ProvObjects::SignupEntry    SE;
        if(!StorageService()->SignupDB().GetRecord("id",SignupUUID,SE)) {
            return NotFound();
        }

        if(Operation == "emailVerified") {
            std::cout << "Verified email for : " << SE.email << std::endl;

            SE.info.modified = OpenWifi::Now();
            SE.status = "emailVerified";
            StorageService()->SignupDB().UpdateRecord("id", SE.info.id, SE);

            Poco::JSON::Object  Answer;
            SE.to_json(Answer);
            return ReturnObject(Answer);
        }

    }

}