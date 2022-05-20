//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#pragma once

#include "framework/MicroService.h"
#include "framework/StorageClass.h"

#include "storage/storage_entity.h"
#include "storage/storage_policies.h"
#include "storage/storage_venue.h"
#include "storage/storage_location.h"
#include "storage/storage_op_locations.h"
#include "storage/storage_contact.h"
#include "storage/storage_op_contacts.h"
#include "storage/storage_inventory.h"
#include "storage/storage_management_roles.h"
#include "storage/storage_configurations.h"
#include "storage/storage_tags.h"
#include "storage/storage_maps.h"
#include "storage/storage_signup.h"
#include "storage/storage_variables.h"
#include "storage/storage_operataor.h"
#include "storage/storage_service_class.h"
#include "storage/storage_sub_devices.h"

namespace OpenWifi {

    class Storage : public StorageClass {
        public:
            static auto instance() {
                static auto instance_ = new Storage;
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
            OpenWifi::MapDB & MapDB() { return *MapDB_; };
            OpenWifi::SignupDB & SignupDB() { return *SignupDB_; };
            OpenWifi::VariablesDB & VariablesDB() { return *VariablesDB_; };
            OpenWifi::OperatorDB & OperatorDB() { return *OperatorDB_; };
            OpenWifi::ServiceClassDB & ServiceClassDB() { return *ServiceClassDB_; };
            OpenWifi::SubscriberDeviceDB & SubscriberDeviceDB() { return *SubscriberDeviceDB_; };
            OpenWifi::OpLocationDB & OpLocationDB() { return *OpLocationDB_; };
            OpenWifi::OpContactDB & OpContactDB() { return *OpContactDB_; };

            bool Validate(const Poco::URI::QueryParameters &P, RESTAPI::Errors::msg &Error);
            bool Validate(const Types::StringVec &P, std::string &Error);
            inline bool ValidatePrefix(const std::string &P) const { return ExistFunc_.find(P)!=ExistFunc_.end(); }
            bool ExpandInUse(const Types::StringVec &UUIDs, ExpandedListMap & Map, std::vector<std::string> & Errors);
            bool ValidateSingle(const std::string &P, std::string & Error);
            bool Validate(const std::string &P);

            void onTimer(Poco::Timer & timer);

            inline const std::string & DefaultOperator() { return DefaultOperator_; }

            static inline bool ApplyRules(const ProvObjects::DeviceRules &R, ProvObjects::DeviceRules & R_res) {
                if(R_res.rcOnly=="inherit")
                    R_res.rcOnly=R.rcOnly;
                if(R_res.firmwareUpgrade=="inherit")
                    R_res.firmwareUpgrade=R.firmwareUpgrade;
                if(R_res.rrm=="inherit")
                    R_res.rrm=R.rrm;

                return (R_res.rrm=="inherit" || R_res.rcOnly=="inherit" || R_res.firmwareUpgrade=="inherit");
            }

            static inline bool ApplyConfigRules(ProvObjects::DeviceRules & R_res) {
                if(R_res.firmwareUpgrade=="inherit")
                    R_res.firmwareUpgrade=MicroService::instance().ConfigGetString("firmware.updater.upgrade","yes");
                if(R_res.rcOnly=="inherit")
                    R_res.rcOnly=MicroService::instance().ConfigGetString("firmware.updater.releaseonly","yes");
                if(R_res.rrm=="inherit")
                    R_res.rrm=MicroService::instance().ConfigGetString("rrm.default","no");
                return true;
            }

    private:
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
            std::unique_ptr<OpenWifi::MapDB>                    MapDB_;
            std::unique_ptr<OpenWifi::SignupDB>                 SignupDB_;
            std::unique_ptr<OpenWifi::VariablesDB>              VariablesDB_;
            std::unique_ptr<OpenWifi::OperatorDB>               OperatorDB_;
            std::unique_ptr<OpenWifi::ServiceClassDB>           ServiceClassDB_;
            std::unique_ptr<OpenWifi::SubscriberDeviceDB>       SubscriberDeviceDB_;
            std::unique_ptr<OpenWifi::OpLocationDB>             OpLocationDB_;
            std::unique_ptr<OpenWifi::OpContactDB>              OpContactDB_;
            std::string                                         DefaultOperator_;


            typedef std::function<bool(const char *FieldName, std::string &Value)>   exist_func;
            typedef std::function<bool(const char *FieldName, std::string &Value, std::string &Name, std::string &Description)>   expand_func;
            std::map<std::string, exist_func>                   ExistFunc_;
            std::map<std::string, expand_func>                  ExpandFunc_;
            Poco::Timer                                         Timer_;
            std::unique_ptr<Poco::TimerCallback<Storage>>       TimerCallback_;

            void ConsistencyCheck();
            void InitializeSystemDBs();

   };

   inline auto StorageService() { return Storage::instance(); }

}  // namespace

