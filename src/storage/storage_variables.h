//
// Created by stephane bourque on 2022-02-23.
//

#pragma once

#include "RESTObjects/RESTAPI_ProvObjects.h"
#include "framework/orm.h"

namespace OpenWifi {
	typedef Poco::Tuple<std::string, std::string, std::string, std::string, uint64_t, uint64_t,
						std::string, std::string, std::string, std::string, std::string,
						std::string, std::string>
		VariablesDBRecordType;

	class VariablesDB : public ORM::DB<VariablesDBRecordType, ProvObjects::VariableBlock> {
	  public:
		explicit VariablesDB(OpenWifi::DBType T, Poco::Data::SessionPool &P,
							 Poco::Logger &L) noexcept;
		virtual ~VariablesDB(){};

	  private:
		bool Upgrade(uint32_t from, uint32_t &to) override;
	};
} // namespace OpenWifi
