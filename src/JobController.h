//
// Created by stephane bourque on 2021-10-28.
//

#pragma once

#include <vector>
#include <utility>
#include <functional>
#include <list>
#include "framework/MicroService.h"

namespace OpenWifi {

    class Job : public Poco::Runnable {
    public:
        Job(const std::string &JobID, const std::string &name, const std::vector<std::string> & parameters, uint64_t when, const SecurityObjects::UserInfo &UI, Poco::Logger &L) :
            jobId_(JobID),
            name_(name),
            parameters_(parameters),
            when_(when),
            userinfo_(UI),
            Logger_(L)
        {};

        virtual void run() = 0;
        [[nodiscard]] std::string Name() const { return name_; }
        const SecurityObjects::UserInfo & UserInfo() const { return userinfo_; }
        Poco::Logger & Logger() { return Logger_; }
        const std::string & JobId() const { return jobId_; }
        const std::string & Parameter(int x) const { return parameters_[x];}
        uint64_t When() const { return when_; }
        void Start() { started_ = OpenWifi::Now(); }
        uint64_t Started() const { return started_; }
        uint64_t Completed() const { return completed_;}
        void Complete() { completed_ = OpenWifi::Now(); }

    private:
        std::string                 jobId_;
        std::string                 name_;
        std::vector<std::string>    parameters_;
        uint64_t                    when_=0;
        SecurityObjects::UserInfo   userinfo_;
        Poco::Logger                & Logger_;
        uint64_t                    started_=0;
        uint64_t                    completed_=0;
    };

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

            void AddJob( Job* newJob ) {
                std::lock_guard G(Mutex_);

                jobs_.push_back(newJob);

            }

        private:
            Poco::Thread                        Thr_;
            std::atomic_bool                    Running_=false;
            std::list<Job *>                    jobs_;
            Poco::ThreadPool                    Pool_;

        JobController() noexcept:
            SubSystemServer("JobController", "JOB-SVR", "job")
            {
            }
    };
    inline auto JobController() { return JobController::instance(); }

}

