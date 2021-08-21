//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "Poco/Util/Application.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/JSON/Parser.h"

#include "StorageService.h"
#include "Daemon.h"
#include "Utils.h"
#include "OpenAPIRequest.h"

namespace OpenWifi {

	class Storage *Storage::instance_ = nullptr;

	Storage::Storage() noexcept:
	  SubSystemServer("Storage", "STORAGE-SVR", "storage")
    {
    }

    int Storage::Start() {
		SubMutexGuard		Guard(Mutex_);

		Logger_.setLevel(Poco::Message::PRIO_NOTICE);
        Logger_.notice("Starting.");
        std::string DBType = Daemon()->ConfigGetString("storage.type");

        if (DBType == "sqlite") {
            DBType_ = ORM::DBType::sqlite;
            Setup_SQLite();
        } else if (DBType == "postgresql") {
            DBType_ = ORM::DBType::postgresql;
            Setup_PostgreSQL();
        } else if (DBType == "mysql") {
            DBType_ = ORM::DBType::mysql;
            Setup_MySQL();
        }

        EntityDB_ = std::make_unique<OpenWifi::EntityDB>(DBType_,*Pool_, Logger_);
        PolicyDB_ = std::make_unique<OpenWifi::PolicyDB>(DBType_, *Pool_, Logger_);
        VenueDB_ = std::make_unique<OpenWifi::VenueDB>(DBType_, *Pool_, Logger_);
        LocationDB_ = std::make_unique<OpenWifi::LocationDB>(DBType_, *Pool_, Logger_);
        ContactDB_ = std::make_unique<OpenWifi::ContactDB>(DBType_, *Pool_, Logger_);
        InventoryDB_ = std::make_unique<OpenWifi::InventoryDB>(DBType_, *Pool_, Logger_);
        RolesDB_ = std::make_unique<OpenWifi::ManagementRoleDB>(DBType_, *Pool_, Logger_);

        EntityDB_->Create();
        PolicyDB_->Create();
        VenueDB_->Create();
        LocationDB_->Create();
        ContactDB_->Create();
        InventoryDB_->Create();
        RolesDB_->Create();

        OpenWifi::ProvObjects::Entity   R;
        EntityDB_->GetRecord("id","xxx",R);

        Updater_.start(*this);
        return 0;
    }

    void Storage::run() {
	    Running_ = true ;
	    bool FirstRun=true;
	    uint64_t Retry = 10000;
	    while(Running_) {
	        if(!FirstRun)
	            Poco::Thread::trySleep(DeviceTypes_.empty() ? 2000 : 60000);
	        if(!Running_)
	            break;
	        if(UpdateDeviceTypes())
	            FirstRun = false;
	    }
	}


    /*  Get the device types... /api/v1/firmwares?deviceSet=true
     {
          "deviceTypes": [
            "cig_wf160d",
            "cig_wf188",
            "cig_wf194c",
            "edgecore_eap101",
            "edgecore_eap102",
            "edgecore_ecs4100-12ph",
            "edgecore_ecw5211",
            "edgecore_ecw5410",
            "edgecore_oap100",
            "edgecore_spw2ac1200",
            "edgecore_ssw2ac2600",
            "indio_um-305ac",
            "linksys_e8450-ubi",
            "linksys_ea8300",
            "mikrotik_nand",
            "mikrotik_nand-large",
            "tplink_cpe210_v3",
            "tplink_cpe510_v3",
            "tplink_eap225_outdoor_v1",
            "tplink_ec420",
            "tplink_ex227",
            "tplink_ex228",
            "tplink_ex447",
            "wallys_dr40x9"
          ]
        }
     */
	bool Storage::UpdateDeviceTypes() {

	    try {
	        Types::StringPairVec QueryData;

	        QueryData.push_back(std::make_pair("deviceSet","true"));
	        OpenAPIRequestGet	Req(    uSERVICE_FIRMWARE,
                                     "/api/v1/firmwares",
                                     QueryData,
                                     5000);

	        Poco::JSON::Object::Ptr Response;
	        auto StatusCode = Req.Do(Response);
	        if( StatusCode == Poco::Net::HTTPResponse::HTTP_OK) {
	            if(Response->isArray("deviceTypes")) {
	                SubMutexGuard G(Mutex_);
	                DeviceTypes_.clear();
	                auto Array = Response->getArray("deviceTypes");
	                for(const auto &i:*Array) {
	                    DeviceTypes_.insert(i.toString());
	                }
	                return true;
	            }
	        } else {
	        }
	    } catch (const Poco::Exception &E) {
	        Logger_.log(E);
	    }
	    return false;
	}

    void Storage::Stop() {
	    Running_=false;
	    Updater_.wakeUp();
	    Updater_.join();
        Logger_.notice("Stopping.");
    }

    bool Storage::Validate(const Poco::URI::QueryParameters &P, std::string &Error) {
	    for(const auto &i:P) {
	        if(i.first == "addContact" || i.first == "delContact") {
	            if(!ContactDB_->Exists("id",i.second)) {
	                Error = "Unknown contact UUID: " + i.second;
	                break;
	            }
	        }
	        if(i.first == "addLocation" || i.first == "delLocation") {
	            if(!LocationDB_->Exists("id",i.second)) {
	                Error = "Unknown Location UUID: " + i.second;
	                break;
	            }
	        }
	        if(i.first == "addEntity" || i.first == "delEntity") {
	            if(!EntityDB_->Exists("id",i.second)) {
	                Error = "Unknown Entity UUID: " + i.second;
	                break;
	            }
	        }
	        if(i.first == "addVenue" || i.first == "delVenue") {
	            if(!VenueDB_->Exists("id",i.second)) {
	                Error = "Unknown Venue UUID: " + i.second;
	                break;
	            }
	        }
	        if(i.first == "addManager" || i.first == "delManager") {
	            /*
	            if(!VenueDB_->Exists("id",i.second)) {
	                Error = "Unknown Manager UUID: " + i.second;
	                break;
	            }*/
	        }
	        if(i.first == "addDevice" || i.first == "delDevice") {
	            if(!InventoryDB_->Exists("id",i.second)) {
	                Error = "Unknown Inventory UUID: " + i.second;
	                break;
	            }
	        }
	    }

	    if(Error.empty())
	        return true;
	    return false;
	}

}

// namespace