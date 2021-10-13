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
#include "RESTAPI_ProvObjects.h"

namespace OpenWifi {

	class Storage *Storage::instance_ = nullptr;

    int Storage::Start() {
		std::lock_guard		Guard(Mutex_);

		Logger_.setLevel(Poco::Message::PRIO_NOTICE);
        Logger_.notice("Starting.");
        std::string DBType = Daemon()->ConfigGetString("storage.type");

        if (DBType == "sqlite") {
            Setup_SQLite();
        } else if (DBType == "postgresql") {
            Setup_PostgreSQL();
        } else if (DBType == "mysql") {
            Setup_MySQL();
        }

        EntityDB_ = std::make_unique<OpenWifi::EntityDB>(dbType_,*Pool_, Logger_);
        PolicyDB_ = std::make_unique<OpenWifi::PolicyDB>(dbType_, *Pool_, Logger_);
        VenueDB_ = std::make_unique<OpenWifi::VenueDB>(dbType_, *Pool_, Logger_);
        LocationDB_ = std::make_unique<OpenWifi::LocationDB>(dbType_, *Pool_, Logger_);
        ContactDB_ = std::make_unique<OpenWifi::ContactDB>(dbType_, *Pool_, Logger_);
        InventoryDB_ = std::make_unique<OpenWifi::InventoryDB>(dbType_, *Pool_, Logger_);
        RolesDB_ = std::make_unique<OpenWifi::ManagementRoleDB>(dbType_, *Pool_, Logger_);
        ConfigurationDB_ = std::make_unique<OpenWifi::ConfigurationDB>(dbType_, *Pool_, Logger_);
        TagsDictionaryDB_ = std::make_unique<OpenWifi::TagsDictionaryDB>(dbType_, *Pool_, Logger_);
        TagsObjectDB_ = std::make_unique<OpenWifi::TagsObjectDB>(dbType_, *Pool_, Logger_);

        EntityDB_->Create();
        PolicyDB_->Create();
        VenueDB_->Create();
        LocationDB_->Create();
        ContactDB_->Create();
        InventoryDB_->Create();
        RolesDB_->Create();
        ConfigurationDB_->Create();
        TagsDictionaryDB_->Create();
        TagsObjectDB_->Create();

        ExistFunc_[EntityDB_->Prefix()] = [=](const char *F, std::string &V) -> bool { return EntityDB_->Exists(F,V); };
        ExistFunc_[PolicyDB_->Prefix()] = [=](const char *F, std::string &V) -> bool { return PolicyDB_->Exists(F,V); };
        ExistFunc_[VenueDB_->Prefix()] = [=](const char *F, std::string &V) -> bool { return VenueDB_->Exists(F,V); };
        ExistFunc_[ContactDB_->Prefix()] = [=](const char *F, std::string &V) -> bool { return ContactDB_->Exists(F,V); };
        ExistFunc_[InventoryDB_->Prefix()] = [=](const char *F, std::string &V) -> bool { return InventoryDB_->Exists(F,V); };
        ExistFunc_[ConfigurationDB_->Prefix()] = [=](const char *F, std::string &V) -> bool { return ConfigurationDB_->Exists(F,V); };
        ExistFunc_[LocationDB_->Prefix()] = [=](const char *F, std::string &V) -> bool { return LocationDB_->Exists(F,V); };
        ExistFunc_[RolesDB_->Prefix()] = [=](const char *F, std::string &V) -> bool { return RolesDB_->Exists(F,V); };
        ExistFunc_[SecurityDBProxy()->Prefix()] = [=](const char *F, std::string &V) ->bool { return SecurityDBProxy()->Exists(F,V); };
        ExistFunc_[TagsDictionaryDB_->Prefix()] = [=](const char *F, std::string &V) ->bool { return TagsDictionaryDB_->Exists(F,V); };
        ExistFunc_[TagsObjectDB_->Prefix()] = [=](const char *F, std::string &V) ->bool { return TagsObjectDB_->Exists(F,V); };

        ExpandFunc_[EntityDB_->Prefix()] = [=](const char *F, std::string &V, std::string &Name, std::string & Description) -> bool { return EntityDB_->GetNameAndDescription(F,V, Name, Description); };
        ExpandFunc_[PolicyDB_->Prefix()] = [=](const char *F, std::string &V, std::string &Name, std::string & Description) -> bool { return PolicyDB_->GetNameAndDescription(F,V, Name, Description); };
        ExpandFunc_[VenueDB_->Prefix()] = [=](const char *F, std::string &V, std::string &Name, std::string & Description) -> bool { return VenueDB_->GetNameAndDescription(F,V, Name, Description); };
        ExpandFunc_[ContactDB_->Prefix()] = [=](const char *F, std::string &V, std::string &Name, std::string & Description) -> bool { return ContactDB_->GetNameAndDescription(F,V, Name, Description); };
        ExpandFunc_[InventoryDB_->Prefix()] = [=](const char *F, std::string &V, std::string &Name, std::string & Description) -> bool { return InventoryDB_->GetNameAndDescription(F,V, Name, Description); };
        ExpandFunc_[ConfigurationDB_->Prefix()] = [=](const char *F, std::string &V, std::string &Name, std::string & Description) -> bool { return ConfigurationDB_->GetNameAndDescription(F,V, Name, Description); };
        ExpandFunc_[LocationDB_->Prefix()] = [=](const char *F, std::string &V, std::string &Name, std::string & Description) -> bool { return LocationDB_->GetNameAndDescription(F,V, Name, Description); };
        ExpandFunc_[RolesDB_->Prefix()] = [=](const char *F, std::string &V, std::string &Name, std::string & Description) -> bool { return RolesDB_->GetNameAndDescription(F,V, Name, Description); };
        ExpandFunc_[SecurityDBProxy()->Prefix()] = [=](const char *F, std::string &V, std::string &Name, std::string & Description) ->bool { return SecurityDBProxy()->Exists(F,V); };
        ExpandFunc_[TagsDictionaryDB_->Prefix()] = [=](const char *F, std::string &V, std::string &Name, std::string & Description) ->bool { return TagsDictionaryDB_->Exists(F,V); };
        ExpandFunc_[TagsObjectDB_->Prefix()] = [=](const char *F, std::string &V, std::string &Name, std::string & Description) ->bool { return TagsObjectDB_->Exists(F,V);; };

        EntityDB_->CheckForRoot();
        Updater_.start(*this);

        return 0;
    }

    void Storage::run() {
	    Running_ = true ;
	    bool FirstRun=true;
	    long Retry = 2000;
	    while(Running_) {
	        if(!FirstRun)
	            Poco::Thread::trySleep(Retry);
	        if(!Running_)
	            break;
	        if(UpdateDeviceTypes()) {
	            FirstRun = false;
	            Logger_.information("Updated existing DeviceType list from FMS.");
	            Retry = 60 * 5 * 1000 ; // 5 minutes
	        } else {
	            Retry = 2000;
	        }
	    }
	}

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
	                std::lock_guard G(Mutex_);
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
	        auto uuid_parts = Utils::Split(i.second,':');
	        if(uuid_parts.size()==2) {
	            auto F = ExistFunc_.find(uuid_parts[0]);
	            if(F!=ExistFunc_.end()) {
	                if(!F->second("id", uuid_parts[1])) {
	                    Error = "Unknown " + F->first + " UUID:" + uuid_parts[1] ;
	                    break;
	                }
	            }
	        }
	    }
	    if(Error.empty())
	        return true;
	    return false;
	}

	bool Storage::ValidateSingle(const std::string &P, std::string & Error) {
        auto uuid_parts = Utils::Split(P,':');
        if(uuid_parts.size()==2) {
            auto F = ExistFunc_.find(uuid_parts[0]);
            if(F!=ExistFunc_.end()) {
                if(!F->second("id", uuid_parts[1])) {
                    Error = "Unknown " + F->first + " UUID:" + uuid_parts[1] ;
                    return false;
                } else {
                    return true;
                }
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    bool Storage::Validate(const Types::StringVec &P, std::string &Error) {
        for(const auto &i:P) {
            if(!ValidateSingle(i,Error))
                return false;
        }
        if(Error.empty())
            return true;
        return false;
	}

	bool Storage::Validate(const std::string &P) {
        std::string Error;
        return ValidateSingle(P,Error);
    }

/*    bool Storage::DeleteContact(const std::string &P, const std::string &Prefix, const std::string &Id) {
        auto uuid_parts = Utils::Split(P,':');
        if(uuid_parts.size()!=2)
            return false;
        if(uuid_parts[0]=="ent") {
            return EntityDB_->DeleteContact("id",uuid_parts[1],Prefix,Id);
        } else if(uuid_parts[0]=="ven") {
            return VenueDB_->DeleteContact("id",uuid_parts[1],Prefix,Id);
        }
        return false;
    }
*/

    bool Storage::ExpandInUse(const Types::StringVec &UUIDs, ExpandedListMap &Map, std::vector<std::string> & Errors) {
        for(const auto &i:UUIDs) {
            auto uuid_parts = Utils::Split(i,':');
            if(uuid_parts.size()==2) {
                auto F = ExpandFunc_.find(uuid_parts[0]);
                if(F!=ExpandFunc_.end()) {
                    std::string Name, Description;
                    if(!F->second("id", uuid_parts[1], Name, Description)) {
                        Errors.push_back(i);
                    } else {
                        auto Hint = Map.find(uuid_parts[0]);
                        ProvObjects::ExpandedUseEntry   X{.uuid=uuid_parts[1],.name=Name, .description=Description};
                        if(Hint==Map.end()) {
                            ProvObjects::ExpandedUseEntryList   L;
                            L.type = uuid_parts[0];
                            L.entries.push_back(X);
                            Map[uuid_parts[0]] = L;
                        } else {
                            Hint->second.entries.push_back(X);
                        }
                    }
                }
            }
        }
        return true;
    }

}

// namespace