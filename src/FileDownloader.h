//
// Created by stephane bourque on 2022-03-11.
//

#pragma once
#include "framework/MicroService.h"
#include "Poco/Timer.h"

namespace OpenWifi {

    class FileDownloader : public SubSystemServer {
    public:

        static auto instance() {
            static auto instance_ = new FileDownloader;
            return instance_;
        }

        int Start() override;
        void Stop() override;
        void onTimer(Poco::Timer & timer);

    private:
        Poco::Timer                                         Timer_;
        std::unique_ptr<Poco::TimerCallback<FileDownloader>>       TimerCallback_;
        std::atomic_bool Running_ = false;

        FileDownloader() noexcept:
                SubSystemServer("FileDownloader", "FILE-DOWNLOADER", "downloader") {
        }
    };

    inline auto FileDownloader() { return FileDownloader::instance(); }
}
