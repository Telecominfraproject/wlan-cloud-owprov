//
// Created by stephane bourque on 2022-02-22.
//

#pragma once

#include "framework/MicroService.h"

namespace OpenWifi {

    class Signup : public SubSystemServer {
    public:

        static auto instance() {
            static auto instance_ = new Signup;
            return instance_;
        }

        int Start() override;
        void Stop() override;

        inline uint64_t GracePeriod() const { return GracePeriod_; };
        inline uint64_t LingerPeriod() const { return LingerPeriod_; }

    private:
        uint64_t    GracePeriod_ = 60 * 60;
        uint64_t    LingerPeriod_ = 24 * 60 * 60;

        Signup() noexcept:
                SubSystemServer("SignupServer", "SIGNUP", "signup")
        {
        }
    };

    inline auto Signup() { return Signup::instance(); }

}
