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

namespace uCentral {

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

}

// namespace