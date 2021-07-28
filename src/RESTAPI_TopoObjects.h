//
// Created by stephane bourque on 2021-07-25.
//

#ifndef UCENTRAL_TOPO_RESTAPI_TOPOOBJECTS_H
#define UCENTRAL_TOPO_RESTAPI_TOPOOBJECTS_H

#include "Poco/JSON/Object.h"

namespace uCentral::TopoObjects {
    struct Report {

        void to_json(Poco::JSON::Object &Obj) const;
        void reset();
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };
}

#endif //UCENTRAL_TOPO_RESTAPI_TOPOOBJECTS_H
