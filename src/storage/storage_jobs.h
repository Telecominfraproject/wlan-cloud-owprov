//
// Created by stephane bourque on 2021-10-28.
//

#pragma once

#include "framework/orm.h"
#include "JobController.h"

namespace OpenWifi {
    typedef Poco::Tuple<
        std::string,
        std::string,
        std::string,
        std::string,
        uint64_t,
        uint64_t,
        std::string
    > JobDBRecordType;

    struct JobRecord {
        Types::UUID_t       id;
        std::string         name;
        std::string         description;
        std::string         type;
        uint64_t            progress;
        uint64_t            total;
        Job::ParametersVec  parameters;

        // void from_string(const std::string &S);
        // std::string to_string() const;
    };

    class JobDB : public ORM::DB<JobDBRecordType, JobRecord> {
    public:
        JobDB( OpenWifi::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L);
    private:
    };
}
