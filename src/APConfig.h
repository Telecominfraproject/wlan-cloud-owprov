//
// Created by stephane bourque on 2021-09-07.
//

#pragma once

#include "Poco/Logger.h"
#include "RESTObjects//RESTAPI_ProvObjects.h"
#include <string>

namespace OpenWifi {

	constexpr std::uint64_t MaximumPossibleRadios = 6;

	struct VerboseElement {
		ProvObjects::DeviceConfigurationElement element;
		ProvObjects::ObjectInfo info;
	};
	typedef std::vector<VerboseElement> ConfigVec;

	class APConfig {
	  public:
		explicit APConfig(const std::string &SerialNumber, const std::string &DeviceType,
						  Poco::Logger &L, bool Explain = false);
		explicit APConfig(const std::string &SerialNumber, Poco::Logger &L);

		[[nodiscard]] bool Get(Poco::JSON::Object::Ptr &Configuration);

		void AddConfiguration(const std::string &UUID);
		void AddConfiguration(const Types::UUIDvec_t &UUID);
		void AddConfiguration(const ProvObjects::DeviceConfigurationElementVec &Elements);
		void AddVenueConfig(const std::string &UUID);
		void AddEntityConfig(const std::string &UUID);
		const Poco::JSON::Array &Explanation() { return Explanation_; };

	  private:
		std::string SerialNumber_;
		std::string DeviceType_;
		Poco::Logger &Logger_;
		std::string CompleteConfig_;
		ConfigVec Config_;
		Types::StringPairVec Errors;
		bool Explain_ = false;
		Poco::JSON::Array Explanation_;
		bool Sub_ = false;
		Poco::Logger &Logger() { return Logger_; }

		bool ReplaceVariablesInArray(const Poco::JSON::Array &O,
									 Poco::JSON::Array &Result);
		void ReplaceNestedVariables(const std::string uuid, Poco::JSON::Object &Result);
		bool ReplaceVariablesInObject(const Poco::JSON::Object &Original,
									  Poco::JSON::Object &Result);

		bool FindRadio(const std::string &Band, const Poco::JSON::Array::Ptr &Arr,
					   Poco::JSON::Object::Ptr &Radio);
		bool mergeArray(const std::string &K, const Poco::JSON::Array::Ptr &A,
						const Poco::JSON::Array::Ptr &B, Poco::JSON::Array &Arr);
		bool merge(const Poco::JSON::Object::Ptr &A, const Poco::JSON::Object::Ptr &B,
				   Poco::JSON::Object::Ptr &C);
		bool RemoveBand(const std::string &Band, const Poco::JSON::Array::Ptr &A_in,
						Poco::JSON::Array::Ptr &A_Out);
        bool InsertRadiusEndPoint(const ProvObjects::RADIUSEndPoint &EP, Poco::JSON::Object &Result);
	};
} // namespace OpenWifi
