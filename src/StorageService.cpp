//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "Poco/Util/Application.h"

#include "StorageService.h"
#include "Daemon.h"
#include "Utils.h"

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
        std::string DBType = uCentral::Daemon()->ConfigGetString("storage.type");

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

        EntityDB_->Create();
        PolicyDB_->Create();
        VenueDB_->Create();
        LocationDB_->Create();
        ContactDB_->Create();
        InventoryDB_->Create();

        OpenWifi::ProvObjects::Entity   R;
        EntityDB_->GetRecord("id","xxx",R);

		return 0;
    }

    void Storage::Stop() {
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