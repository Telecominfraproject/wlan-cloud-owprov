//
// Created by stephane bourque on 2021-09-07.
//

#include "APConfig.h"
#include "StorageService.h"

#include "Poco/JSON/Parser.h"
#include "Poco/StringTokenizer.h"
#include "fmt/format.h"

#include <RadiusEndpointTypes/OrionWifi.h>
#include <RadiusEndpointTypes/GlobalReach.h>

namespace OpenWifi {

	APConfig::APConfig(const std::string &SerialNumber, const std::string &DeviceType,
					   Poco::Logger &L, bool Explain)
		: SerialNumber_(SerialNumber), DeviceType_(DeviceType), Logger_(L), Explain_(Explain) {}

	APConfig::APConfig(const std::string &SerialNumber, Poco::Logger &L)
		: SerialNumber_(SerialNumber), Logger_(L) {
		Explain_ = false;
		Sub_ = true;
	}

	bool APConfig::FindRadio(const std::string &Band, const Poco::JSON::Array::Ptr &Arr,
							 Poco::JSON::Object::Ptr &Radio) {
		for (const auto &i : *Arr) {
			auto R = i.extract<Poco::JSON::Object::Ptr>();
			if (R->has("band") && R->get("band").toString() == Band) {
				Radio = R;
				return true;
			}
		}
		return false;
	}

	bool APConfig::RemoveBand(const std::string &Band, const Poco::JSON::Array::Ptr &A_in,
							  Poco::JSON::Array::Ptr &A_Out) {
		for (const auto &i : *A_in) {
			auto R = i.extract<Poco::JSON::Object::Ptr>();
			if (R->has("band") && R->get("band").toString() == Band) {
			} else {
				A_Out->add(i);
			}
		}
		return false;
	}

	[[maybe_unused]] static void ShowJSON([[maybe_unused]] const char *S,
										  [[maybe_unused]] const Poco::JSON::Object::Ptr &Obj) {
		/*
		std::stringstream O;
		Poco::JSON::Stringifier::stringify(Obj,O);
		std::cout << S << ":" << std::endl;
		std::cout << ">>>" << std::endl << O.str() << std::endl << "<<<" << std::endl;
		 */
	}

    bool APConfig::InsertRadiusEndPoint(const ProvObjects::RADIUSEndPoint &RE, Poco::JSON::Object::Ptr &Result) {
        DBGLINE
        if(RE.UseGWProxy) {
            DBGLINE
            Poco::JSON::Object  ServerSettings;
            if (RE.Type == "orion") {
                DBGLINE
                return OpenRoaming_Orion()->Render(RE, Result);
            } else if (RE.Type == "globalreach") {
                DBGLINE
                return OpenRoaming_GlobalReach()->Render(RE, Result);
            } else if (RE.Type == "radsec") {
                DBGLINE

            } else if (RE.Type == "radius") {
                DBGLINE

            }
            Result->set( "radius" , ServerSettings);
        }
        DBGLINE
        return false;
    }

    bool APConfig::ReplaceVariablesInObject(const Poco::JSON::Object::Ptr &Original,
											Poco::JSON::Object::Ptr &Result) {
		// get all the names and expand
		auto Names = Original->getNames();
		for (const auto &i : Names) {
            DBGLINE
            if (i == "__variableBlock") {
                if (Original->isArray(i)) {
                    auto UUIDs = Original->getArray(i);
                    for (const auto &uuid: *UUIDs) {
                        ProvObjects::VariableBlock VB;
                        if (StorageService()->VariablesDB().GetRecord("id", uuid, VB)) {
                            for (const auto &var: VB.variables) {
                                Poco::JSON::Parser P;
                                auto VariableBlockInfo =
                                        P.parse(var.value).extract<Poco::JSON::Object::Ptr>();
                                auto VarNames = VariableBlockInfo->getNames();
                                for (const auto &j: VarNames) {
                                    std::cout << "Name: " << j << std::endl;
                                    Poco::JSON::Object::Ptr InnerEval;
                                    if(VariableBlockInfo->isArray(j)) {
                                        auto Arr = VariableBlockInfo->getArray(j);
//                                        ReplaceVariablesInObject(,InnerEval);
  //                                      Result->set(j, InnerEval);
                                        std::cout << "Array!!!" << std::endl;
                                    } else if(VariableBlockInfo->isObject(j)) {
                                        std::cout << "Visiting object " << j << std::endl;
                                        auto O = VariableBlockInfo->getObject(j);
                                        ReplaceVariablesInObject(O,InnerEval);
                                        Result->set(j, InnerEval);
                                    } else {
                                        Result->set(j, VariableBlockInfo->get(j));
                                    }
                                }
                            }
                        }
                    }
                }
            } else if (i == "__radiusEndpoint") {
                DBGLINE
                auto EndPointId = Original->get("__radiusEndpoint").toString();
                DBGLINE
                ProvObjects::RADIUSEndPoint RE;
                DBGLINE
                if(!StorageService()->RadiusEndpointDB().GetRecord("id",EndPointId,RE)) {
                    DBGLINE
                    InsertRadiusEndPoint(RE, Result);
                    DBGLINE
                } else {
                    DBGLINE
                    poco_error(Logger_, fmt::format("RADIUS Endpoint {} could not be found. Please delete this configuration and recreate it."));
                    return false;
                }
                DBGLINE
			} else if (Original->isArray(i)) {
                DBGLINE
				auto Arr = Poco::makeShared<Poco::JSON::Array>();
				auto Obj = Original->getArray(i);
				ReplaceVariablesInArray(Obj, Arr);
				Result->set(i, Arr);
                DBGLINE
			} else if (Original->isObject(i)) {
                DBGLINE
				auto Expanded = Poco::makeShared<Poco::JSON::Object>();
				auto Obj = Original->getObject(i);
				ReplaceVariablesInObject(Obj, Expanded);
				Result->set(i, Expanded);
                DBGLINE
			} else {
                DBGLINE
				Result->set(i, Original->get(i));
                DBGLINE
			}
		}
		return true;
	}

	bool APConfig::ReplaceVariablesInArray(const Poco::JSON::Array::Ptr &Original,
										   Poco::JSON::Array::Ptr &ResultArray) {

		for (const auto &element : *Original) {
            std::cout << element.toString() << std::endl;
			if (element.isArray()) {
				auto Expanded = Poco::makeShared<Poco::JSON::Array>();
				const auto &Object = element.extract<Poco::JSON::Array::Ptr>();
				ReplaceVariablesInArray(Object, Expanded);
				ResultArray->add(Expanded);
			} else if (element.isStruct()) {
				auto Expanded = Poco::makeShared<Poco::JSON::Object>();
				const auto &Object = element.extract<Poco::JSON::Object::Ptr>();
				ReplaceVariablesInObject(Object, Expanded);
				ResultArray->add(Expanded);
			} else if (element.isString() || element.isNumeric() || element.isBoolean() ||
					   element.isInteger() || element.isSigned()) {
				ResultArray->add(element);
			} else {
				auto Expanded = Poco::makeShared<Poco::JSON::Object>();
				const auto &Object = element.extract<Poco::JSON::Object::Ptr>();
				ReplaceVariablesInObject(Object, Expanded);
				ResultArray->add(Expanded);
			}
		}
		return true;
	}

	bool APConfig::Get(Poco::JSON::Object::Ptr &Configuration) {
		if (Config_.empty()) {
			Explanation_.clear();
			try {
				if (!Sub_) {
					ProvObjects::InventoryTag D;
					if (StorageService()->InventoryDB().GetRecord("serialNumber", SerialNumber_,
																  D)) {
						if (!D.deviceConfiguration.empty()) {
							// std::cout << "Adding device specific configuration: " << D.deviceConfiguration.size() << std::endl;
							AddConfiguration(D.deviceConfiguration);
						} else {
							// std::cout << "No device specific configuration." << std::endl;
						}
						if (!D.entity.empty()) {
							AddEntityConfig(D.entity);
						} else if (!D.venue.empty()) {
							AddVenueConfig(D.venue);
						}
					}
				} else {
					ProvObjects::SubscriberDevice D;
					if (StorageService()->SubscriberDeviceDB().GetRecord("serialNumber",
																		 SerialNumber_, D)) {
						if (!D.configuration.empty()) {
							AddConfiguration(D.configuration);
						}
					}
				}
				//  Now we have all the config we need.
			} catch (const Poco::Exception &E) {
				Logger_.log(E);
			}
		}

		try {
			std::set<std::string> Sections;
			for (const auto &i : Config_) {
				Poco::JSON::Parser P;
				auto O = P.parse(i.element.configuration).extract<Poco::JSON::Object::Ptr>();
				auto Names = O->getNames();
				for (const auto &SectionName : Names) {
					auto InsertInfo = Sections.insert(SectionName);
					if (InsertInfo.second) {
						if (O->isArray(SectionName)) {
							auto OriginalArray = O->getArray(SectionName);
							if (Explain_) {
								Poco::JSON::Object ExObj;
								ExObj.set("from-uuid", i.info.id);
								ExObj.set("from-name", i.info.name);
								ExObj.set("action", "added");
								ExObj.set("element", OriginalArray);
								Explanation_.add(ExObj);
							}
							auto ExpandedArray = Poco::makeShared<Poco::JSON::Array>();
							ReplaceVariablesInArray(OriginalArray, ExpandedArray);
							Configuration->set(SectionName, ExpandedArray);
						} else if (O->isObject(SectionName)) {
							auto OriginalSection =
								O->get(SectionName).extract<Poco::JSON::Object::Ptr>();
							if (Explain_) {
								Poco::JSON::Object ExObj;
								ExObj.set("from-uuid", i.info.id);
								ExObj.set("from-name", i.info.name);
								ExObj.set("action", "added");
								ExObj.set("element", OriginalSection);
								Explanation_.add(ExObj);
							}
							auto ExpandedSection = Poco::makeShared<Poco::JSON::Object>();
							ReplaceVariablesInObject(OriginalSection, ExpandedSection);
							Configuration->set(SectionName, ExpandedSection);
						} else {
                            poco_warning(Logger(), fmt::format("Unknown config element type: {}",O->get(SectionName).toString()));
						}
					} else {
						if (Explain_) {
							Poco::JSON::Object ExObj;
							ExObj.set("from-uuid", i.info.id);
							ExObj.set("from-name", i.info.name);
							ExObj.set("action", "ignored");
							ExObj.set("reason", "weight insufficient");
							ExObj.set("element", O->get(SectionName));
							Explanation_.add(ExObj);
						}
					}
				}
			}

			//  Apply overrides...
			ProvObjects::ConfigurationOverrideList COL;
			if (StorageService()->OverridesDB().GetRecord("serialNumber", SerialNumber_, COL)) {
				for (const auto &col : COL.overrides) {
					const auto Tokens = Poco::StringTokenizer(col.parameterName, ".");
					if (Tokens[0] == "radios" && Tokens.count() == 3) {
						std::uint64_t RadioIndex = std::strtoull(Tokens[1].c_str(), nullptr, 10);
						if (RadioIndex < MaximumPossibleRadios) {
							auto RadioArray = Configuration->getArray("radios");
							if (RadioIndex < RadioArray->size()) {
								auto IndexedRadio =
									RadioArray->get(RadioIndex).extract<Poco::JSON::Object::Ptr>();
								if (Tokens[2] == "tx-power") {
									IndexedRadio->set(
										"rx-power",
										std::strtoull(col.parameterValue.c_str(), nullptr, 10));
									if (Explain_) {
										Poco::JSON::Object ExObj;
										ExObj.set("from-name", "overrides");
										ExObj.set("override", col.parameterName);
										ExObj.set("source", col.source);
										ExObj.set("reason", col.reason);
										ExObj.set("value", col.parameterValue);
										Explanation_.add(ExObj);
									}
									RadioArray->set(RadioIndex, IndexedRadio);
									Configuration->set("radios", RadioArray);
								} else if (Tokens[2] == "channel") {
									if (col.parameterValue == "auto") {
										IndexedRadio->set("channel", "auto");
									} else {
										IndexedRadio->set(
											"channel",
											std::strtoull(col.parameterValue.c_str(), nullptr, 10));
									}
									// std::cout << "Setting channel in radio " << RadioIndex << std::endl;
									if (Explain_) {
										Poco::JSON::Object ExObj;
										ExObj.set("from-name", "overrides");
										ExObj.set("override", col.parameterName);
										ExObj.set("source", col.source);
										ExObj.set("reason", col.reason);
										ExObj.set("value", col.parameterValue);
										Explanation_.add(ExObj);
									}
									RadioArray->set(RadioIndex, IndexedRadio);
									Configuration->set("radios", RadioArray);
								} else {
									poco_error(
										Logger(),
										fmt::format("{}: Unsupported override variable name {}",
													col.parameterName));
								}
							}
						} else {
							poco_error(Logger(), fmt::format("{}: radio index out of range in {}",
															 col.parameterName));
						}
					} else {
						poco_error(Logger(),
								   fmt::format("{}: Unsupported override variable name {}",
											   col.parameterName));
					}
				}
			}
		} catch (...) {
		}
		return !Config_.empty();
	}

	static bool DeviceTypeMatch(const std::string &DeviceType, const Types::StringVec &Types) {
		for (const auto &i : Types) {
			if (i == "*" || Poco::icompare(DeviceType, i) == 0)
				return true;
		}
		return false;
	}

	void APConfig::AddConfiguration(const ProvObjects::DeviceConfigurationElementVec &Elements) {
		for (const auto &i : Elements) {
			if (i.weight == 0) {
				VerboseElement VE{.element = i, .info = ProvObjects::ObjectInfo{}};
				Config_.push_back(VE);
			} else {
				// we need to insert after everything bigger or equal
				auto Hint = std::lower_bound(Config_.cbegin(), Config_.cend(), i.weight,
											 [](const VerboseElement &Elem, uint64_t Value) {
												 return Elem.element.weight >= Value;
											 });
				VerboseElement VE{.element = i, .info = ProvObjects::ObjectInfo{}};
				Config_.insert(Hint, VE);
			}
		}
	}

	void APConfig::AddConfiguration(const Types::UUIDvec_t &UUIDs) {
		for (const auto &i : UUIDs)
			AddConfiguration(i);
	}

	void APConfig::AddConfiguration(const std::string &UUID) {
		if (UUID.empty())
			return;

		ProvObjects::DeviceConfiguration Config;
		if (StorageService()->ConfigurationDB().GetRecord("id", UUID, Config)) {
			if (!Config.configuration.empty()) {
				if (DeviceTypeMatch(DeviceType_, Config.deviceTypes)) {
					for (const auto &i : Config.configuration) {
						if (i.weight == 0) {
							VerboseElement VE{.element = i, .info = Config.info};
							Config_.push_back(VE);
						} else {
							// we need to insert after everything bigger or equal
							auto Hint =
								std::lower_bound(Config_.cbegin(), Config_.cend(), i.weight,
												 [](const VerboseElement &Elem, uint64_t Value) {
													 return Elem.element.weight >= Value;
												 });
							VerboseElement VE{.element = i, .info = Config.info};
							Config_.insert(Hint, VE);
						}
					}
				} else {
					Poco::JSON::Object ExObj;
					ExObj.set("from-uuid", Config.info.id);
					ExObj.set("from-name", Config.info.name);
					ExObj.set("action", "ignored");
					ExObj.set("reason", "deviceType mismatch");
					Explanation_.add(ExObj);
				}
			} else {
				poco_error(Logger(),
						   fmt::format("Device configuration for {} is empty.", SerialNumber_));
			}
		} else {
			poco_error(Logger(),
					   fmt::format("Invalid device configuration UUID for {}.", SerialNumber_));
		}
	}

	void APConfig::AddEntityConfig(const std::string &UUID) {
		ProvObjects::Entity E;
		if (StorageService()->EntityDB().GetRecord("id", UUID, E)) {
			AddConfiguration(E.configurations);
			if (!E.parent.empty()) {
				AddEntityConfig(E.parent);
			}
		} else {
		}
	}

	void APConfig::AddVenueConfig(const std::string &UUID) {
		ProvObjects::Venue V;
		if (StorageService()->VenueDB().GetRecord("id", UUID, V)) {
			AddConfiguration(V.configurations);
			if (!V.entity.empty()) {
				AddEntityConfig(V.entity);
			} else if (!V.parent.empty()) {
				AddVenueConfig(V.parent);
			}
		} else {
		}
	}
} // namespace OpenWifi