/*******************************************************************************
 * Copyright (c) 2016 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Jeffrey Dare            - initial implementation
 *******************************************************************************/

#ifndef GATEWAYCLIENT_H_
#define GATEWAYCLIENT_H_

#include "MQTTClient.h"
#include <ctype.h>
#define BUFFER_SIZE 1000

// all failure return codes must be negative(extending from mqttclient)
enum errorCodes { CONFIG_FILE_ERROR = -3, MISSING_INPUT_PARAM = -4, QUICKSTART_NOT_SUPPORTED = -5 };

//configuration file structure
struct config {
	char org[15];
	char type[50];
	char id[50];
	char authmethod[10];
	char authtoken[50];
};

//gatewayclient
struct gatewayclient
{
	Network n;
	Client c;
	struct config config;

	unsigned char buf[BUFFER_SIZE];
    unsigned char readbuf[BUFFER_SIZE];
};

typedef struct gatewayclient GatewayClient;

//Callback used to process commands
typedef void (*commandCallback)(char* type, char* id, char* commandName, char *format, void* payload, size_t payloadlen);

/**
* Function used to initialize the Watson IoT Gateway client
* @param client - Reference to the GatewayClient
* @param org - Your organization ID
* @param type - The type of your Gateway
* @param id - The ID of your Gateway
* @param auth-method - Method of authentication (the only value currently supported is “token”)
* @param auth-token - API key token (required if auth-method is “token”)
*
* @return int return code
*/
int initializeGateway(GatewayClient *client, char *orgId, char *gwType, char *gwId, char *authmethod, char *authtoken);
/**
* Function used to initialize the Watson IoT Gateway client using the config file which is generated when you register your device
* @param client - Reference to the GatewayClient
* @param configFilePath - File path to the configuration file 
*
* @return int return code
* error codes
* CONFIG_FILE_ERROR -3 - Config file not present or not in right format
*/
int initializeGateway_configfile(GatewayClient *client, char *configFilePath);

/**
* Function used to Connect the Watson IoT Gateway client
* @param client - Reference to the GatewayClient
*
* @return int return code
*/
int connectGateway(GatewayClient *client);

/**
* Function used to Publish events from the device to the Watson IoT
* @param client - Reference to the GatewayClient
* @param eventType - Type of event to be published e.g status, gps
* @param eventFormat - Format of the event e.g json
* @param data - Payload of the event
* @param QoS - qos for the publish event. Supported values : QOS0, QOS1, QOS2
*
* @return int return code from the publish
*/
int publishGatewayEvent(GatewayClient *client, char *eventType, char *eventFormat, unsigned char* data, enum QoS qos);

/**
* Function used to Publish events from the device to the Watson IoT
* @param client - Reference to the GatewayClient
* @param deviceType - The type of your device
* @param deviceId - The ID of your deviceId
* @param eventType - Type of event to be published e.g status, gps
* @param eventFormat - Format of the event e.g json
* @param data - Payload of the event
* @param QoS - qos for the publish event. Supported values : QOS0, QOS1, QOS2
*
* @return int return code from the publish
*/
int publishDeviceEvent(GatewayClient *client, char *deviceType, char *deviceId, char *eventType, char *eventFormat, unsigned char* data, enum QoS qos);

/**
* Function used to set the Command Callback function. This must be set if you to recieve commands.
* @param client - Reference to the GatewayClient
*
* @param cb - A Function pointer to the commandCallback. Its signature - void (*commandCallback)(char* commandName, char* payload)
* @return int return code
*/
void setGatewayCommandHandler(GatewayClient *client, commandCallback cb);

/**
* Function used to subscribe to all commands for the Gateway.
* @param client - Reference to the GatewayClient
*
* @return int return code
*/
int subscribeToGatewayCommands(GatewayClient *client);

/**
* Function used to subscribe to device commands in a  gateway.
*
* @return int return code
*/
int subscribeToDeviceCommands(GatewayClient *client, char* deviceType, char* deviceId, char* command, char* format, int qos) ;

/**
* Function used to check if the client is connected
* @param client - Reference to the GatewayClient
*
* @return int return code
*/
int isGatewayConnected(GatewayClient *client);

/**
* Function used to Yield for commands.
* @param client - Reference to the GatewayClient
* @param time_ms - Time in milliseconds
* @return int return code
*/
int gatewayYield(GatewayClient *client, int time_ms);

/**
* Function used to disconnect from the IBM Watson IoT
* @param client - Reference to the GatewayClient
*
* @return int return code
*/
int disconnectGateway(GatewayClient *client);

#endif /* GATEWAYCLIENT_H_ */
