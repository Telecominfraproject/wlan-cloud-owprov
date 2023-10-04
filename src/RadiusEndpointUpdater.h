//
// Created by stephane bourque on 2023-10-02.
//

#pragma once
#include <framework/AppServiceRegistry.h>
#include <framework/utils.h>

/*

                "request-attribute": {
                    "type": "array",
                    "items": {
                        "anyOf": [
                            {
                                "type": "object",
                                "properties": {
                                    "vendor-id": {
                                        "type": "integer",
                                        "maximum": 65535,
                                        "minimum": 1
                                    },
                                    "vendor-attributes": {
                                        "type": "array",
                                        "items": {
                                            "type": "object",
                                            "properties": {
                                                "id": {
                                                    "type": "integer",
                                                    "maximum": 255,
                                                    "minimum": 1
                                                },
                                                "value": {
                                                    "type": "string"
                                                }
                                            }
                                        }
                                    }
                                }
                            },
                            {
                                "type": "object",
                                "properties": {
                                    "id": {
                                        "type": "integer",
                                        "maximum": 255,
                                        "minimum": 1
                                    },
                                    "value": {
                                        "type": "integer",
                                        "maximum": 4294967295,
                                        "minimum": 0
                                    }
                                },
                                "examples": [
                                    {
                                        "id": 27,
                                        "value": 900
                                    },
                                    {
                                        "id": 56,
                                        "value": 1004
                                    }
                                ]
                            },
                            {
                                "type": "object",
                                "properties": {
                                    "id": {
                                        "type": "integer",
                                        "maximum": 255,
                                        "minimum": 1
                                    },
                                    "value": {
                                        "type": "string"
                                    }
                                },
                                "examples": [
                                    {
                                        "id": 32,
                                        "value": "My NAS ID"
                                    },
                                    {
                                        "id": 126,
                                        "value": "Example Operator"
                                    }
                                ]
                            },
                            {
                                "type": "object",
                                "properties": {
                                    "id": {
                                        "type": "integer",
                                        "maximum": 255,
                                        "minimum": 1
                                    },
                                    "hex-value": {
                                        "type": "string"
                                    }
                                },
                                "examples": [
                                    {
                                        "id": 32,
                                        "value": "0a0b0c0d"
                                    }
                                ]
                            }
                        ]
                    }
                }


 */
#include <string>

namespace OpenWifi {
    class RadiusEndpointUpdater {
    public:
        inline bool UpdateEndpoints( [[maybe_unused]] std::string & Error,
                                     [[maybe_unused]] uint64_t &ErrorNum  ) {

            AppServiceRegistry().Set("radiusEndpointLastUpdate", Utils::Now());
            return false;
        }
    private:

    };
} // OpenWifi
