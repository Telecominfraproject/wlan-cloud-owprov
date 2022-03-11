//
// Created by stephane bourque on 2022-03-11.
//

#include "FileDownloader.h"
#include "Daemon.h"

namespace OpenWifi {
    int FileDownloader::Start() {
        TimerCallback_ = std::make_unique<Poco::TimerCallback<FileDownloader>>(*this,&FileDownloader::onTimer);
        Timer_.setStartInterval( 20 * 1000);  // first run in 20 seconds
        Timer_.setPeriodicInterval(2 * 60 * 60 * 1000); // 1 hours
        Timer_.start(*TimerCallback_);
        return 0;
    }

    void FileDownloader::Stop() {
        Timer_.stop();
        Logger().notice("Stopping.");
    }

    void FileDownloader::onTimer(Poco::Timer &timer) {
        const static std::vector<std::pair<std::string,std::string>> Files
            {
                {"https://raw.githubusercontent.com/blogic/ucentral-schema/main/ucentral.schema.json", "ucentral.schema.json" },
                {"https://ucentral.io/ucentral.schema.pretty.json", "ucentral.schema.pretty.json" }
            };

        for(const auto &[url,filename]:Files) {
            try {
                std::string FileContent;
                if (Utils::wgets(url, FileContent)) {
                    std::ofstream OF(Daemon()->AssetDir() + "/" + filename,
                                     std::ios_base::out | std::ios_base::trunc);
                    OF << FileContent;
                    Logger().warning(Poco::format("File %s was downloaded",url));
                }
            } catch(...) {
                Logger().warning(Poco::format("File %s could not be downloaded",url));
            }
        }
    }
}