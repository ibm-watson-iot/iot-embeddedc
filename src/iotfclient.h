/*******************************************************************************
 * Copyright (c) 2015 IBM Corp.
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
 *    Jeffrey Dare            - initial implementation and API implementation
 *    Sathiskumar Palaniappan - Added support to create multiple Iotfclient
 *                              instances within a single process
 *******************************************************************************/

#ifndef IOTCLIENT_H_
#define IOTCLIENT_H_

#include "MQTTClient.h"
#include <ctype.h>

#define BUFFER_SIZE 1024

// all failure return codes must be negative(extending from mqttclient)
enum errorCodes { CONFIG_FILE_ERROR = -3, MISSING_INPUT_PARAM = -4 };

//configuration file structure
struct config {
	char org[15];
	char type[50];
	char id[50];
	char authmethod[10];
	char authtoken[50];
};

//iotfclient
struct iotfclient
{
	Network n;
	Client c;
	struct config config;

	unsigned char buf[BUFFER_SIZE];
        unsigned char readbuf[BUFFER_SIZE];
        int isQuickstart;
};

typedef struct iotfclient Iotfclient;

#define Iotfclient_initializer { NULL, DefaultClient, {"", "", "", "", ""} }

//Callback used to process commands
typedef void (*commandCallback)(char* commandName, char *format, void* payload);

/**
* Function used to initialize the Watson IoT client
* @param client - Reference to the Iotfclient
* @param org - Your organization ID
* @param type - The type of your device
* @param id - The ID of your device
* @param auth-method - Method of authentication (the only value currently supported is â€œtokenâ€�)
* @param auth-token - API key token (required if auth-method is â€œtokenâ€�)
*
* @return int return code
*/
int initialize(Iotfclient *client, char *orgId, char *deviceType, char *deviceId, char *authmethod, char *authtoken);
/**
* Function used to initialize the IBM Watson IoT client using the config file which is generated when you register your device
* @param client - Reference to the Iotfclient
* @param configFilePath - File path to the configuration file 
*
* @return int return code
* error codes
* CONFIG_FILE_ERROR -3 - Config file not present or not in right format
*/
int initialize_configfile(Iotfclient *client, char *configFilePath);

/**
* Function used to initialize the IBM Watson IoT client
* @param client - Reference to the Iotfclient
*
* @return int return code
*/
int connectiotf(Iotfclient *client);

/**
* Function used to Publish events from the device to the IBM Watson IoT service
* @param client - Reference to the Iotfclient
* @param eventType - Type of event to be published e.g status, gps
* @param eventFormat - Format of the event e.g json
* @param data - Payload of the event
* @param QoS - qos for the publish event. Supported values : QOS0, QOS1, QOS2
*
* @return int return code from the publish
*/
int publishEvent(Iotfclient *client, char *eventType, char *eventFormat, unsigned char* data, enum QoS qos);

/**
* Function used to set the Command Callback function. This must be set if you to recieve commands.
* @param client - Reference to the Iotfclient
*
* @param cb - A Function pointer to the commandCallback. Its signature - void (*commandCallback)(char* commandName, char* payload)
* @return int return code
*/
void setCommandHandler(Iotfclient *client, commandCallback cb);

/**
* Function used to subscribe to all commands. This function is by default called when in registered mode.
* @param client - Reference to the Iotfclient
*
* @return int return code
*/
int subscribeCommands(Iotfclient *client);

/**
* Function used to check if the client is connected
* @param client - Reference to the Iotfclient
*
* @return int return code
*/
int isConnected(Iotfclient *client);

/**
* Function used to Yield for commands.
* @param client - Reference to the Iotfclient
* @param time_ms - Time in milliseconds
* @return int return code
*/
int yield(Iotfclient *client, int time_ms);

/**
* Function used to disconnect from the IBM Watson IoT service
* @param client - Reference to the Iotfclient
*
* @return int return code
*/
int disconnect(Iotfclient *client);

#endif /* IOTCLIENT_H_ */
