//
// Created by stephane bourque on 2023-09-11.
//

//
// Created by stephane bourque on 2023-09-11.
//


#pragma once

#include "RESTObjects/RESTAPI_ProvObjects.h"
#include "framework/orm.h"

namespace OpenWifi {

    typedef Poco::Tuple<
            std::string,
            std::string,
            std::string,
            std::string,
            std::string,
            std::string,
            std::string,
            uint64_t,
            uint64_t>
            GLBLRCertsDBRecordType;

    class GLBLRCertsDB : public ORM::DB<GLBLRCertsDBRecordType, ProvObjects::GLBLRCertificateInfo> {
    public:
        GLBLRCertsDB(OpenWifi::DBType T, Poco::Data::SessionPool &P, Poco::Logger &L);
        virtual ~GLBLRCertsDB(){};
        bool Upgrade(uint32_t from, uint32_t &to) override;
    private:

    };
} // namespace OpenWifi
