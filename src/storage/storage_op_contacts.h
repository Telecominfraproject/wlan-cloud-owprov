//
// Created by stephane bourque on 2022-04-07.
//

#include "framework/orm.h"
#include "RESTObjects/RESTAPI_ProvObjects.h"

namespace OpenWifi {
    typedef Poco::Tuple<
            std::string,
            std::string,
            std::string,
            std::string,
            uint64_t,
            uint64_t,
            std::string,
            std::string,
            std::string,
            std::string,
            std::string,
            std::string,
            std::string,
            std::string,
            std::string,
            std::string,
            std::string,
            std::string,
            std::string,
            std::string,
            std::string,
            std::string
    > OpContactDBRecordType;

    class OpContactDB : public ORM::DB<OpContactDBRecordType, ProvObjects::OperatorContact> {
    public:
        OpContactDB( OpenWifi::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L);
        virtual ~OpContactDB() {};
    private:
    };
}
