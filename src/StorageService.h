//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRAL_USTORAGESERVICE_H
#define UCENTRAL_USTORAGESERVICE_H

#include <map>

#include "Poco/Data/Session.h"
#include "Poco/Data/SessionPool.h"
#include "Poco/Data/SQLite/Connector.h"

#include "Poco/Data/PostgreSQL/Connector.h"
#include "Poco/Data/MySQL/Connector.h"
#include "Poco/URI.h"
#include "SubSystemServer.h"

#include "Storage.h"
#include "storage_entity.h"
#include "storage_policies.h"
#include "storage_venue.h"
#include "storage_location.h"
#include "storage_contact.h"
#include "storage_inventory.h"
#include "storage_management_roles.h"
#include "storage_configurations.h"
#include "storage_tags.h"

#include "SecurityDBProxy.h"

namespace OpenWifi {

    class Storage : public SubSystemServer, Poco::Runnable {
        public:
            static Storage *instance() {
                if (instance_ == nullptr) {
                    instance_ = new Storage;
                }
                return instance_;
            }

            int 	Start() override;
            void 	Stop() override;

            typedef std::list<ProvObjects::ExpandedUseEntry>                    ExpandedInUseList;
            typedef std::map<std::string, ProvObjects::ExpandedUseEntryList>    ExpandedListMap;

            OpenWifi::EntityDB & EntityDB() { return *EntityDB_; };
            OpenWifi::PolicyDB & PolicyDB() { return *PolicyDB_; };
            OpenWifi::VenueDB & VenueDB() { return *VenueDB_; };
            OpenWifi::LocationDB & LocationDB() { return *LocationDB_; };
            OpenWifi::ContactDB & ContactDB() { return *ContactDB_;};
            OpenWifi::InventoryDB & InventoryDB() { return *InventoryDB_; };
            OpenWifi::ManagementRoleDB & RolesDB() { return *RolesDB_; };
            OpenWifi::ConfigurationDB & ConfigurationDB() { return *ConfigurationDB_; };
            OpenWifi::TagsDictionaryDB & TagsDictionaryDB() { return *TagsDictionaryDB_; };
            OpenWifi::TagsObjectDB & TagsObjectDB() { return *TagsObjectDB_; };

            bool Validate(const Poco::URI::QueryParameters &P, std::string &Error);
            bool Validate(const Types::StringVec &P, std::string &Error);
            inline bool ValidatePrefix(const std::string &P) const { return ExistFunc_.find(P)!=ExistFunc_.end(); }
            bool ExpandInUse(const Types::StringVec &UUIDs, ExpandedListMap & Map, std::vector<std::string> & Errors);
            bool ValidateSingle(const std::string &P, std::string & Error);
            bool Validate(const std::string &P);

            inline bool IsAcceptableDeviceType(const std::string &D) const { return (DeviceTypes_.find(D)!=DeviceTypes_.end());};
            inline bool AreAcceptableDeviceTypes(const Types::StringVec &S, bool WildCardAllowed=true) const {
                for(const auto &i:S) {
                    if(WildCardAllowed && i=="*") {
                       //   We allow wildcards
                    } else if(DeviceTypes_.find(i)==DeviceTypes_.end())
                        return false;
                }
                return true;
            }

            bool DeleteContact(const std::string &P, const std::string &Prefix, const std::string &Id);

            void run() final;

          private:
            static Storage      								*instance_;
            std::unique_ptr<Poco::Data::SessionPool>        	Pool_;
            std::unique_ptr<Poco::Data::SQLite::Connector>  	SQLiteConn_;
            std::unique_ptr<Poco::Data::PostgreSQL::Connector>  PostgresConn_;
            std::unique_ptr<Poco::Data::MySQL::Connector>       MySQLConn_;
            DBType                                              dbType_ = sqlite;
            std::unique_ptr<OpenWifi::EntityDB>                 EntityDB_;
            std::unique_ptr<OpenWifi::PolicyDB>                 PolicyDB_;
            std::unique_ptr<OpenWifi::VenueDB>                  VenueDB_;
            std::unique_ptr<OpenWifi::LocationDB>               LocationDB_;
            std::unique_ptr<OpenWifi::ContactDB>                ContactDB_;
            std::unique_ptr<OpenWifi::InventoryDB>              InventoryDB_;
            std::unique_ptr<OpenWifi::ManagementRoleDB>         RolesDB_;
            std::unique_ptr<OpenWifi::ConfigurationDB>          ConfigurationDB_;
            std::unique_ptr<OpenWifi::TagsDictionaryDB>         TagsDictionaryDB_;
            std::unique_ptr<OpenWifi::TagsObjectDB>             TagsObjectDB_;

            typedef std::function<bool(const char *FieldName, std::string &Value)>   exist_func;
            typedef std::function<bool(const char *FieldName, std::string &Value, std::string &Name, std::string &Description)>   expand_func;
            std::map<std::string, exist_func>                   ExistFunc_;
            std::map<std::string, expand_func>                  ExpandFunc_;

            Poco::Thread                                        Updater_;
            std::set<std::string>                               DeviceTypes_;
            std::atomic_bool                                    Running_=false;

            bool UpdateDeviceTypes();
            Storage() noexcept:
                SubSystemServer("Storage", "STORAGE-SVR", "storage")
                {
                }

            int 	Setup_SQLite();
            int 	Setup_MySQL();
            int 	Setup_PostgreSQL();
   };

   inline Storage * Storage() { return Storage::instance(); }

}  // namespace

#endif //UCENTRAL_USTORAGESERVICE_H
