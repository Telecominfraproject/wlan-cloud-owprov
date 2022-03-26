//
// Created by stephane bourque on 2021-10-28.
//

#pragma once

#include <vector>
#include <utility>
#include <functional>
#include "framework/MicroService.h"

namespace OpenWifi {

    class Job {
        public:
            struct Parameter {
                std::string name;
                std::string value;
                inline void to_json(Poco::JSON::Object &Obj) const {
                    RESTAPI_utils::field_to_json(Obj,"name",name);
                    RESTAPI_utils::field_to_json(Obj,"value",value);
                }

                inline bool from_json(const Poco::JSON::Object::Ptr &Obj) {
                    try {
                        RESTAPI_utils::field_from_json(Obj,"name",name);
                        RESTAPI_utils::field_from_json(Obj,"value",value);
                        return true;
                    } catch (...) {

                    }
                    return false;
                }
            };

            struct Status {
                Types::UUID_t   UUID;
                uint64_t        Start = 0 ;
                uint64_t        Progress = 0 ;
                uint64_t        Completed = 0 ;
                std::string     CurrentDisplay;
            };

            struct Result {
                int         Error=0;
                std::string Reason;
            };

            typedef std::vector<Parameter>       Parameters;
            typedef std::vector<Parameters>      ParametersVec;
            typedef std::function<bool(const Parameters &Parameters, Result &Result, bool &Retry)>  WorkerFunction;
            typedef std::vector<Status>          Statuses;

            Job(std::string Title,
                         std::string Description,
                         std::string RegisteredName,
                         ParametersVec Parameters,
                         [[maybe_unused]] bool Parallel=true) :
                    Title_(std::move(Title)),
                    Description_(std::move(Description)),
                    RegisteredName_(std::move(RegisteredName)),
                    Parameters_(std::move(Parameters))
                {
                    UUID_ = MicroService::instance().CreateUUID();
                }

            [[nodiscard]] inline const Types::UUID_t & ID() const { return UUID_; }

        private:
            Types::UUID_t       UUID_;
            std::string         Title_;
            std::string         Description_;
            std::string         RegisteredName_;
            ParametersVec       Parameters_;
    };

    class JobRegistry {
        public:
            static auto instance() {
                static auto instance_ = new JobRegistry;
                return instance_;
            }

            inline void RegisterJobType( const std::string & JobType, Job::WorkerFunction Function) {
                    JobTypes_[JobType] = std::move(Function);
            }

            inline bool Execute(const std::string &JobType, const Job::Parameters & Params, Job::Result &Result, bool & Retry) {
                auto Hint = JobTypes_.find(JobType);
                if(Hint != end(JobTypes_)) {
                    Hint->second(Params, Result, Retry);
                    return true;
                }
                return false;
            }

        private:
            std::map<std::string,Job::WorkerFunction>  JobTypes_;
    };

    inline auto JobRegistry() { return JobRegistry::instance(); }

    class JobController : public SubSystemServer, Poco::Runnable {
        public:
            static auto instance() {
                static auto instance_ = new JobController;
                return instance_;
            }

            int Start() override;
            void Stop() override;
            void run() override;
            inline void wakeup() { Thr_.wakeUp(); }

            bool JobList(Job::Statuses & Statuses);

        private:
            Poco::Thread            Thr_;
            std::atomic_bool        Running_=false;

        JobController() noexcept:
            SubSystemServer("JobController", "JOB-SVR", "job")
            {
            }
    };
    inline auto JobController() { return JobController::instance(); }

}

