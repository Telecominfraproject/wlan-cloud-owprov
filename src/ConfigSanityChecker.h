//
// Created by stephane bourque on 2021-10-01.
//

#ifndef OWPROV_CONFIGSANITYCHECKER_H
#define OWPROV_CONFIGSANITYCHECKER_H

#include <string>
#include <list>
#include <functional>
#include <map>
#include <vector>
#include <utility>
#include "nlohmann/json.hpp"

namespace OpenWifi {
    struct SanityError {
        std::string     Cause;
        std::string     Reason;
        std::string     Severity;
    };

    typedef std::list<SanityError>  SanityErrorList;

    class ConfigSanityChecker {
        public:
            explicit ConfigSanityChecker(std::string Config, std::string DeviceType) :
                Config_(std::move(Config)),
                DeviceType_(std::move(DeviceType)){}

            bool Check();
            const SanityErrorList & Errors() { return Errors_; }
            const SanityErrorList & Warnings() { return Warnings_; }

            typedef std::function<void(nlohmann::json &)>   CheckFuncType;

            struct KeyToFunc {
                std::string        Key;
                CheckFuncType      Func;
            };
            typedef std::pair<std::string, CheckFuncType>   FuncPair;
            typedef std::vector<FuncPair>    FuncList;

            void Check_radios(nlohmann::json &);
            void Check_interfaces(nlohmann::json &);
            void Check_metrics(nlohmann::json &);
            void Check_services(nlohmann::json &);
            void Check_uuid(nlohmann::json &);

        private:
            std::string         Config_;
            std::string         DeviceType_;
            SanityErrorList     Errors_;
            SanityErrorList     Warnings_;
            FuncList            Funcs_{
                std::make_pair("radios", [this](nlohmann::json &d){ this->Check_radios(d);} ) ,
                std::make_pair("interfaces", [this](nlohmann::json &d){ this->Check_interfaces(d);} ),
                std::make_pair("metrics", [this](nlohmann::json &d){ this->Check_metrics(d);} ),
                std::make_pair("services", [this](nlohmann::json &d){ this->Check_services(d);} ),
                std::make_pair("uuid", [this](nlohmann::json &d){ this->Check_uuid(d);} )
                };
    };
}

#endif //OWPROV_CONFIGSANITYCHECKER_H
