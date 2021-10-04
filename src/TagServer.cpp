//
// Created by stephane bourque on 2021-10-02.
//

#include "TagServer.h"
#include "StorageService.h"

namespace OpenWifi {
    class TagServer *TagServer::instance_ = nullptr;

    int TagServer::Start() {

        //  we need to get the entire dictionary in memory...
        // std::function<bool(const TagsDictionary &)> Function = [](const TagsDictionary &D) -> bool { return  true; };
        Storage()->TagsDictionaryDB().Iterate([](const TagsDictionary &D) -> bool { return  true; });


        return 0;
    }

    void TagServer::Stop() {

    }

    void TagServer::run() {

    }

}