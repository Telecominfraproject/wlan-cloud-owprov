//
// Created by stephane bourque on 2022-02-22.
//

#pragma once

#include "framework/MicroService.h"
#include "RESTObjects/RESTAPI_ProvObjects.h"

namespace OpenWifi {

    class Signup : public SubSystemServer, Poco::Runnable {
    public:

        static auto instance() {
            static auto instance_ = new Signup;
            return instance_;
        }

        int Start() override;
        void Stop() override;
        void run() final;

        inline uint64_t GracePeriod() const { return GracePeriod_; };
        inline uint64_t LingerPeriod() const { return LingerPeriod_; }

        inline bool GetSignupById(const std::string & UUID, ProvObjects::SignupEntry &SE) {
            std::lock_guard     G(Mutex_);
            auto hint = OutstandingSignups_.find(UUID);

            if(hint == end(OutstandingSignups_))
                return false;

            SE = hint->second;
            return true;
        }

        inline void RemoveSignupById(const std::string & UUID) {
            std::lock_guard     G(Mutex_);
            OutstandingSignups_.erase(UUID);
        }

        inline void AddOutstandingSignup(const ProvObjects::SignupEntry &SE) {
            std::lock_guard     G(Mutex_);
            OutstandingSignups_[SE.info.id] = SE;
        }

    private:
        uint64_t            GracePeriod_ = 60 * 60;
        uint64_t            LingerPeriod_ = 24 * 60 * 60;
        std::atomic_bool    Running_ = false;
        std::map<std::string, ProvObjects::SignupEntry>  OutstandingSignups_;
        Poco::Thread    Worker_;

        Signup() noexcept:
                SubSystemServer("SignupServer", "SIGNUP", "signup")
        {
        }
    };

    inline auto Signup() { return Signup::instance(); }

}
