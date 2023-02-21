//
// Created by stephane bourque on 2022-04-07.
//

#include "RESTObjects/RESTAPI_ProvObjects.h"
#include "framework/orm.h"

namespace OpenWifi {
	typedef Poco::Tuple<std::string, std::string, std::string, std::string, uint64_t, uint64_t,
						std::string, std::string, std::string, std::string, std::string,
						std::string, std::string, std::string, std::string, std::string,
						std::string, std::string, std::string, std::string>
		OpLocationDBRecordType;

	class OpLocationDB : public ORM::DB<OpLocationDBRecordType, ProvObjects::OperatorLocation> {
	  public:
		OpLocationDB(OpenWifi::DBType T, Poco::Data::SessionPool &P, Poco::Logger &L);
		virtual ~OpLocationDB(){};

	  private:
	};
} // namespace OpenWifi
