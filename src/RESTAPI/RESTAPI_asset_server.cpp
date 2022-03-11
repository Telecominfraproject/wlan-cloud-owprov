//
// Created by stephane bourque on 2021-07-10.
//

#include "RESTAPI_asset_server.h"
#include "Poco/File.h"
#include "framework/ow_constants.h"
#include "Daemon.h"

namespace OpenWifi {
    void RESTAPI_asset_server::DoGet() {
        Poco::File  AssetFile;

        std::string AssetName = GetBinding(RESTAPI::Protocol::ID, "");
        AssetFile = Daemon()->AssetDir() + "/" + AssetName;

        if(!AssetFile.isFile()) {
            return NotFound();
        }

        SendFile(AssetFile);
    }
}