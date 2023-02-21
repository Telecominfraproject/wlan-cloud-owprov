//
// Created by stephane bourque on 2021-11-09.
//

#pragma once

#include "RESTObjects/RESTAPI_ProvObjects.h"
#include "framework/orm.h"

namespace OpenWifi {
	typedef Poco::Tuple<std::string, std::string, std::string, std::string, uint64_t, uint64_t,
						std::string, std::string, std::string, std::string, std::string,
						std::string, std::string, std::string>
		MapDBRecordType;

	class MapDB : public ORM::DB<MapDBRecordType, ProvObjects::Map> {
	  public:
		MapDB(OpenWifi::DBType T, Poco::Data::SessionPool &P, Poco::Logger &L);
		virtual ~MapDB(){};

	  private:
		bool Upgrade(uint32_t from, uint32_t &to) override;
	};
} // namespace OpenWifi
