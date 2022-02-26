//
// Created by stephane bourque on 2022-02-23.
//

#include "RESTAPI_variables_list_handler.h"
#include "RESTAPI/RESTAPI_db_helpers.h"

namespace OpenWifi {

    void RESTAPI_variables_list_handler::DoGet() {
        return ListHandler<VariablesDB>("variableBlocks", DB_, *this);
    }

}