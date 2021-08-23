//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//


#include "RESTAPI_utils.h"

namespace OpenWifi::RESTAPI_utils {

	void EmbedDocument(const std::string & ObjName, Poco::JSON::Object & Obj, const std::string &ObjStr) {
		std::string D = ObjStr.empty() ? "{}" : ObjStr;
		Poco::JSON::Parser P;
		Poco::Dynamic::Var result = P.parse(D);
		const auto &DetailsObj = result.extract<Poco::JSON::Object::Ptr>();
		Obj.set(ObjName, DetailsObj);
	}
}

