//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRAL_USTORAGESERVICE_H
#define UCENTRAL_USTORAGESERVICE_H

#include "Poco/Data/Session.h"
#include "Poco/Data/SessionPool.h"
#include "Poco/Data/SQLite/Connector.h"

#ifndef SMALL_BUILD
#include "Poco/Data/PostgreSQL/Connector.h"
#include "Poco/Data/MySQL/Connector.h"
#endif

#include "RESTAPI_TopoObjects.h"
#include "SubSystemServer.h"

namespace uCentral {

    class Storage : public SubSystemServer {

    public:
		enum StorageType {
			sqlite,
			pgsql,
			mysql
		};

		enum CommandExecutionType {
			COMMAND_PENDING,
			COMMAND_EXECUTED,
			COMMAND_COMPLETED
		};

        static Storage *instance() {
            if (instance_ == nullptr) {
                instance_ = new Storage;
            }
            return instance_;
        }


		int Create_Tables();

		int 	Start() override;
		void 	Stop() override;
		int 	Setup_SQLite();
		[[nodiscard]] std::string ConvertParams(const std::string &S) const;

#ifndef SMALL_BUILD
		int 	Setup_MySQL();
		int 	Setup_PostgreSQL();
#endif

	  private:
		static Storage      								*instance_;
		std::unique_ptr<Poco::Data::SessionPool>        	Pool_= nullptr;
		StorageType 										dbType_ = sqlite;
		std::unique_ptr<Poco::Data::SQLite::Connector>  	SQLiteConn_= nullptr;
#ifndef SMALL_BUILD
		std::unique_ptr<Poco::Data::PostgreSQL::Connector>  PostgresConn_= nullptr;
		std::unique_ptr<Poco::Data::MySQL::Connector>       MySQLConn_= nullptr;
#endif

		Storage() noexcept;
   };

   inline Storage * Storage() { return Storage::instance(); }

}  // namespace

#endif //UCENTRAL_USTORAGESERVICE_H
