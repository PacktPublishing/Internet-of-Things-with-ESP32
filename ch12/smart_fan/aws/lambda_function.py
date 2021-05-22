
import logging
import time
import json
import uuid
import boto3
from fan1_responses import *


logger = logging.getLogger()
logger.setLevel(logging.INFO)
client = boto3.client('iot-data')

def lambda_handler(request, context):
    """Lambda handler for the smart home skill
    """

    try:
        logger.info("Directive:")
        logger.info(json.dumps(request, indent=4, sort_keys=True))

        request_namespace = request["directive"]["header"]["namespace"]
        request_name = request["directive"]["header"]["name"]
        corrTkn = ""
        if "correlationToken" in request["directive"]["header"]:
            corrTkn = request["directive"]["header"]["correlationToken"]

        if request_namespace == "Alexa.Discovery" and request_name == "Discover":
            response = gen_discovery_response()
        elif request_namespace == "Alexa.Authorization" and request_name == "AcceptGrant":
            response = gen_acceptgrant_response()
        elif request_namespace == "Alexa.PowerLevelController" and request_name == "SetPowerLevel":
            response = set_power_level(request["directive"]["payload"]["powerLevel"], corrTkn)
        elif request_namespace == "Alexa.PowerLevelController" and request_name == "AdjustPowerLevel":
            response = adj_power_level(request["directive"]["payload"]["powerLevelDelta"], corrTkn)
        elif request_namespace == "Alexa" and request_name == "ReportState":
            response = gen_report_state(corrTkn)
        else:
            logger.error("unexpected request")
            return response

        logger.info("Response:")
        logger.info(json.dumps(response, indent=4, sort_keys=True))

        return response

    except ValueError as error:
        logger.error(error)
        raise

def get_uuid():
    return str(uuid.uuid4())

def get_utc_timestamp(seconds=None):
    return time.strftime("%Y-%m-%dT%H:%M:%S.00Z", time.gmtime(seconds))


def read_power_level_shadow():
    response = client.get_thing_shadow(thingName=endpoint_id)
    streamingBody = response["payload"].read().decode('utf-8')
    jsonState = json.loads(streamingBody)
    return jsonState["state"]["reported"]["powerlevel"]

def set_power_level_shadow(power_level):
    payload = json.dumps({'state': { 'desired': { 'powerlevel': power_level } }})
    response = client.update_thing_shadow(
        thingName = endpoint_id, 
        payload =  payload)
    logger.info("update shadow result: " + response['payload'].read().decode('utf-8'))  

def gen_discovery_response():
    response = discovery_response
    response["event"]["header"]["messageId"] = get_uuid()
    return response

def gen_acceptgrant_response():
    response = accept_grant_response
    response["event"]["header"]["messageId"] = get_uuid()
    return response

def init_response(response, tkn, power_level):
    response["event"]["header"]["messageId"] = get_uuid()
    response["event"]["header"]["correlationToken"] = tkn
    response["context"]["properties"][0]["timeOfSample"] = get_utc_timestamp()
    response["context"]["properties"][0]["value"] = power_level
    return response


def gen_report_state(tkn):
    response = init_response(state_report, tkn, read_power_level_shadow())
    response["context"]["properties"][1]["timeOfSample"] = get_utc_timestamp()

    return response


def set_power_level(power_level, tkn):
    power_level = update_power(power_level)
    set_power_level_shadow(power_level)

    response = init_response(set_power_level_response, tkn, power_level)
    return response

def adj_power_level(power_level_change, tkn):
    power_level = read_power_level_shadow()
    power_level += power_level_change
    
    power_level = update_power(power_level)
    set_power_level_shadow(power_level)

    response = init_response(adjust_power_level_response, tkn, power_level)
    return response

def update_power(power_level):
    if power_level == 0:
        return power_level

    if power_level <= 33:
        power_level = 33
    elif power_level <=66:
        power_level = 66
    else:
        power_level = 100
    return power_level