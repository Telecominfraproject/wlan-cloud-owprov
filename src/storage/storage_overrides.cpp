//
// Created by stephane bourque on 2022-11-03.
//

#include "storage_overrides.h"
#include "SerialNumberCache.h"
#include "framework/OpenWifiTypes.h"
#include "framework/RESTAPI_utils.h"

namespace OpenWifi {
	static ORM::FieldVec OverridesDB_Fields{// object info
											ORM::Field{"serialNumber", 64, true},
											ORM::Field{"managementPolicy", ORM::FieldType::FT_TEXT},
											ORM::Field{"overrides", ORM::FieldType::FT_TEXT}};

	static ORM::IndexVec OverridesDB_Indexes{};

	OverridesDB::OverridesDB(OpenWifi::DBType T, Poco::Data::SessionPool &P, Poco::Logger &L)
		: DB(T, "overrides", OverridesDB_Fields, OverridesDB_Indexes, P, L, "ovr") {}

	bool OverridesDB::Upgrade([[maybe_unused]] uint32_t from, uint32_t &to) {
		to = Version();
		std::vector<std::string> Script{};

		for (const auto &i : Script) {
			try {
				auto Session = Pool_.get();
				Session << i, Poco::Data::Keywords::now;
			} catch (...) {
			}
		}
		return true;
	}
} // namespace OpenWifi
template <>
void ORM::DB<OpenWifi::OverridesDBRecordType, OpenWifi::ProvObjects::ConfigurationOverrideList>::
	Convert(const OpenWifi::OverridesDBRecordType &In,
			OpenWifi::ProvObjects::ConfigurationOverrideList &Out) {
	Out.serialNumber = In.get<0>();
	Out.managementPolicy = In.get<1>();
	Out.overrides =
		OpenWifi::RESTAPI_utils::to_object_array<OpenWifi::ProvObjects::ConfigurationOverride>(
			In.get<2>());
}

template <>
void ORM::DB<OpenWifi::OverridesDBRecordType, OpenWifi::ProvObjects::ConfigurationOverrideList>::
	Convert(const OpenWifi::ProvObjects::ConfigurationOverrideList &In,
			OpenWifi::OverridesDBRecordType &Out) {
	Out.set<0>(In.serialNumber);
	Out.set<1>(In.managementPolicy);
	Out.set<2>(OpenWifi::RESTAPI_utils::to_string(In.overrides));
}
