
endpoint_id = "myhome_fan1"

discovery_response = {
    "event": {
        "header": {
            "namespace": "Alexa.Discovery",
            "name": "Discover.Response",
            "payloadVersion": "3",
            "messageId": "<message id>"
        },
        "payload": {
            "endpoints": [{
                "endpointId": endpoint_id,
                "manufacturerName": "iot-with-esp32",
                "modelName": "Smart fan - v1",
                "description": "Smart fan for your home",
                "friendlyName": "Smart fan",
                "displayCategories": ["FAN"],
                "cookie": {},
                "capabilities": [{
                        "type": "AlexaInterface",
                        "interface": "Alexa.PowerLevelController",
                        "version": "3",
                        "properties": {
                            "supported": [{
                                "name": "powerLevel"
                            }],
                            "proactivelyReported": True,
                            "retrievable": True
                        }
                    },
                    {
                        "type": "AlexaInterface",
                        "interface": "Alexa.EndpointHealth",
                        "version": "3",
                        "properties": {
                            "supported": [{
                                "name": "connectivity"
                            }],
                            "proactivelyReported": True,
                            "retrievable": True
                        }
                    },
                    {
                        "type": "AlexaInterface",
                        "interface": "Alexa",
                        "version": "3"
                    }
                ]
            }]
        }
    }
}

state_report = {
    "event": {
        "header": {
            "namespace": "Alexa",
            "name": "StateReport",
            "messageId": "<message id>",
            "correlationToken": "<an opaque correlation token>",
            "payloadVersion": "3"
        },
        "endpoint": {
            "endpointId": endpoint_id
        },
        "payload": {}
    },
    "context": {
        "properties": [{
                "namespace": "Alexa.PowerLevelController",
                "name": "powerLevel",
                "value": 52,
                "timeOfSample": "2017-02-03T16:20:50.52Z",
                "uncertaintyInMilliseconds": 0
            },
            {
                "namespace": "Alexa.EndpointHealth",
                "name": "connectivity",
                "value": {
                    "value": "OK"
                },
                "timeOfSample": "2017-02-03T16:20:50.52Z",
                "uncertaintyInMilliseconds": 0
            }
        ]
    }
}

accept_grant_response = {
    "event": {
        "header": {
            "namespace": "Alexa.Authorization",
            "name": "AcceptGrant.Response",
            "payloadVersion": "3",
            "messageId": "5f8a426e-01e4-4cc9-8b79-65f8bd0fd8a4"
        },
        "payload": {}
    }
}

set_power_level_response = {
    "event": {
        "header": {
            "namespace": "Alexa",
            "name": "Response",
            "messageId": "<message id>",
            "correlationToken": "<an opaque correlation token>",
            "payloadVersion": "3"
        },
        "endpoint": {
            "endpointId": endpoint_id
        },
        "payload": {}
    },
    "context": {
        "properties": [{
            "namespace": "Alexa.PowerLevelController",
            "name": "powerLevel",
            "value": 40,
            "timeOfSample": "2017-02-03T16:20:50.52Z",
            "uncertaintyInMilliseconds": 500
        }]
    }
}


adjust_power_level_response = {
    "event": {
        "header": {
            "namespace": "Alexa",
            "name": "Response",
            "messageId": "<message id>",
            "correlationToken": "<an opaque correlation token>",
            "payloadVersion": "3"
        },
        "endpoint": {
            "endpointId": endpoint_id
        },
        "payload": {}
    },
    "context": {
        "properties": [{
            "namespace": "Alexa.PowerLevelController",
            "name": "powerLevel",
            "value": 52,
            "timeOfSample": "2017-02-03T16:20:50.52Z",
            "uncertaintyInMilliseconds": 500
        }]
    }
}