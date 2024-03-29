//
// Created by stephane bourque on 2021-10-01.
//

#include "ConfigSanityChecker.h"
#include "nlohmann/json.hpp"
#include <iomanip>
#include <iostream>

namespace OpenWifi {

	bool ConfigSanityChecker::Check() {
		try {
			auto Doc = nlohmann::json::parse(Config_);

			for (const auto &[key, value] : Doc.items()) {
				for (const auto &i : Funcs_)
					if (i.first == key)
						i.second(value);
			}
			return true;
		} catch (...) {
		}
		return false;
	}

	void ConfigSanityChecker::Check_radios([[maybe_unused]] nlohmann::json &d) {
		std::cout << "Validating radios" << std::endl;
	};

	void ConfigSanityChecker::Check_interfaces([[maybe_unused]] nlohmann::json &d) {
		std::cout << "Validating interfaces" << std::endl;
	};

	void ConfigSanityChecker::Check_metrics([[maybe_unused]] nlohmann::json &d) {
		std::cout << "Validating metrics" << std::endl;
	};

	void ConfigSanityChecker::Check_services([[maybe_unused]] nlohmann::json &d) {
		std::cout << "Validating services" << std::endl;
	};

	void ConfigSanityChecker::Check_uuid([[maybe_unused]] nlohmann::json &d) {
		std::cout << "Validating uuid" << std::endl;
	};

} // namespace OpenWifi