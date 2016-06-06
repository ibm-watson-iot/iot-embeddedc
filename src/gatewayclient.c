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

#include <stdio.h>
#include <signal.h>
#include <memory.h>

#include <sys/time.h>
#include "gatewayclient.h"

//Command Callback
commandCallback cb;

//util functions
char *trim(char *str);
int get_config(char * filename, struct config * configstr);
void messageArrived(MessageData* md);
int length(char *str);
int retry_connection(GatewayClient *client);
int reconnect_delay(int i);

char* subscribeTopics[5];
int subscribeCount = 0;

/**
* Function used to initialize the Watson IoT Gateway client using the config file which is generated when you register your device
* @param client - Reference to the GatewayClient
* @param configFilePath - File path to the configuration file 
*
* @return int return code
* error codes
* CONFIG_FILE_ERROR -3 - Config file not present or not in right format
*/
int initializeGateway_configfile(GatewayClient *client, char *configFilePath)
{
	struct config configstr = {"", "", "", "", ""};

	int rc = 0;

	rc = get_config(configFilePath, &configstr);

	if(rc != SUCCESS) {
		return rc;
	}

	if(!length(configstr.org) || !length(configstr.type) || !length(configstr.id) || !length(configstr.authmethod) || !length(configstr.authtoken)) {
		return CONFIG_FILE_ERROR;
	}

	client->config = configstr;

	return rc;

}

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
* error codes
* MISSING_INPUT_PARAM -4 - Mandatory paramters missing
* QUICKSTART_NOT_SUPPORTED -5 - Gateway is not supported in Quickstart
*/
int initializeGateway(GatewayClient *client, char *orgId, char *gwType, char *gwId, char *authmethod, char *authtoken)
{

	struct config configstr = {"", "", "", "", ""};

	if(orgId==NULL || gwType==NULL || gwId==NULL) {
		return MISSING_INPUT_PARAM;
	}

	strncpy(configstr.org, orgId, 15);
	strncpy(configstr.type, gwType, 50);
	strncpy(configstr.id, gwId, 50);

	if((strcmp(orgId,"quickstart") == 0)) {
		printf("Quickstart mode is not supported in Gateway Client\n");
		return QUICKSTART_NOT_SUPPORTED;
	}

	strncpy(configstr.authmethod, authmethod, 10);
	strncpy(configstr.authtoken, authtoken, 50);

	client->config = configstr;

	return SUCCESS;
}

/**
* Function used to Connect the Watson IoT Gateway client
* @param client - Reference to the GatewayClient
*
* @return int return code
*/
int connectGateway(GatewayClient *client)
{

	int rc = 0;

	const char* messagingUrl = ".messaging.internetofthings.ibmcloud.com";

	char hostname[strlen(client->config.org) + strlen(messagingUrl) + 1];
	
	sprintf(hostname, "%s%s", client->config.org, messagingUrl);

    //TODO : change to 8883 if registered, add support when available in MQTTClient
    int port = 1883;

    char clientId[strlen(client->config.org) + strlen(client->config.type) + strlen(client->config.id) + 5];
    sprintf(clientId, "g:%s:%s:%s", client->config.org, client->config.type, client->config.id);

	NewNetwork(&client->n);
	ConnectNetwork(&client->n, hostname, port);
	MQTTClient(&client->c, &client->n, 1000, client->buf, BUFFER_SIZE, client->readbuf, BUFFER_SIZE);
 
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;       
	data.willFlag = 0;
	data.MQTTVersion = 3;
	data.clientID.cstring = clientId;

	printf("Connecting to registered service with org %s\n", client->config.org);
	data.username.cstring = "use-token-auth";
	data.password.cstring = client->config.authtoken;

	data.keepAliveInterval = 10;
	data.cleansession = 1;
	
	rc = MQTTConnect(&client->c, &data);

	//Subscibe to all commands in gateway by default
	subscribeToGatewayCommands(client);

	return rc;
}

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
int publishDeviceEvent(GatewayClient *client, char *deviceType, char *deviceId, char *eventType, char *eventFormat, unsigned char* data, enum QoS qos)
{
	int rc = -1;

	char publishTopic[strlen(eventType) + strlen(eventFormat) + strlen(deviceType) + strlen(deviceId)+25];

	sprintf(publishTopic, "iot-2/type/%s/id/%s/evt/%s/fmt/%s", deviceType, deviceId, eventType, eventFormat);
	
	MQTTMessage pub;

	pub.qos = qos;
	pub.retained = '0';
	pub.payload = data;
	pub.payloadlen = strlen(data);

	rc = MQTTPublish(&client->c, publishTopic , &pub);

	if(rc != SUCCESS) {
		printf("connection lost.. %d \n",rc);
		retry_connection(client);
		rc = MQTTPublish(&client->c, publishTopic , &pub);
	}
	
	return rc;

}

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
int publishGatewayEvent(GatewayClient *client, char *eventType, char *eventFormat, unsigned char* data, enum QoS qos)
{
	int rc = -1;

	char publishTopic[strlen(eventType) + strlen(eventFormat) + strlen(client->config.id) + strlen(client->config.type)+25];

	sprintf(publishTopic, "iot-2/type/%s/id/%s/evt/%s/fmt/%s", client->config.type, client->config.id, eventType, eventFormat);
	MQTTMessage pub;

	pub.qos = qos;
	pub.retained = '0';
	pub.payload = data;
	pub.payloadlen = strlen(data);

	rc = MQTTPublish(&client->c, publishTopic , &pub);

	if(rc != SUCCESS) {
		printf("connection lost.. \n");
		retry_connection(client);
		rc = MQTTPublish(&client->c, publishTopic , &pub);
	}
	
	return rc;

}

/**
* Function used to set the Command Callback function. This must be set if you to recieve commands.
*
* @param cb - A Function pointer to the commandCallback. Its signature - void (*commandCallback)(char* commandName, char* payload)
* @return int return code
*/
void setGatewayCommandHandler(GatewayClient *client, commandCallback handler)
{
	cb = handler;
}

/**
* Function used to subscribe to all commands for gateway.
*
* @return int return code
*/
int subscribeToGatewayCommands(GatewayClient *client) 
{
	int rc = -1;

	char* subscribeTopic = NULL;

	subscribeTopic = (char*) malloc(strlen(client->config.id) + strlen(client->config.type) + 28);

	sprintf(subscribeTopic, "iot-2/type/%s/id/%s/cmd/+/fmt/+", client->config.type, client->config.id);

	rc = MQTTSubscribe(&client->c, subscribeTopic, QOS2, messageArrived);

	subscribeTopics[subscribeCount++] = subscribeTopic;

	return rc;
}

/**
* Function used to subscribe to device commands for gateway.
*
* @return int return code
*/
int subscribeToDeviceCommands(GatewayClient *client, char* deviceType, char* deviceId, char* command, char* format, int qos) 
{
	int rc = -1;

	char* subscribeTopic = NULL;

	subscribeTopic = (char*)malloc(strlen(deviceType) + strlen(deviceId) + strlen(command) + strlen(format) + 26);

	sprintf(subscribeTopic, "iot-2/type/%s/id/%s/cmd/%s/fmt/%s", deviceType, deviceId, command, format);

	rc = MQTTSubscribe(&client->c, subscribeTopic, qos, messageArrived);

	subscribeTopics[subscribeCount++] = subscribeTopic;
	
	return rc;
}

/**
* Function used to Yield for commands.
* @param time_ms - Time in milliseconds
* @return int return code
*/
int gatewayYield(GatewayClient *client, int time_ms)
{
	int rc = 0;
	rc = MQTTYield(&client->c, time_ms);
	return rc;
}

/**
* Function used to check if the client is connected
*
* @return int return code
*/
int isGatewayConnected(GatewayClient *client)
{
	return client->c.isconnected;
}

/**
* Function used to disconnect from the IoTF service
*
* @return int return code
*/

int disconnectGateway(GatewayClient *client)
{
	int rc = 0;
	int count;
	rc = MQTTDisconnect(&client->c);
	client->n.disconnect(&client->n);

	//free memory
	for(count = 0; count < subscribeCount ; count++) {
		char* topic = subscribeTopics[count];
		free(topic);
	}
	

	return rc;

}

//Handler for all commands. Invoke the callback.
void messageArrived(MessageData* md)
{
	if(cb != 0) {
		MQTTMessage* message = md->message;

		char *topic = malloc(md->topicName->lenstring.len+1);

		sprintf(topic,"%.*s",md->topicName->lenstring.len,md->topicName->lenstring.data);

		void *payload = message->payload;

		size_t payloadlen = message->payloadlen;
		 
		strtok(topic, "/");
		strtok(NULL, "/");

		char *type = strtok(NULL, "/");
		strtok(NULL, "/");
		char *id = strtok(NULL, "/");
		strtok(NULL, "/");
		char *commandName = strtok(NULL, "/");
		strtok(NULL, "/");
		char *format = strtok(NULL, "/");

		free(topic);

		(*cb)(type,id,commandName, format, payload,payloadlen);
	}
}


//Utility Functions

//Trimming characters
char *trim(char *str) {
	size_t len = 0;
	char *frontp = str - 1;
	char *endp = NULL;

	if (str == NULL)
		return NULL;

	if (str[0] == '\0')
		return str;

	len = strlen(str);
	endp = str + len;

	while (isspace(*(++frontp)))
		;
	while (isspace(*(--endp)) && endp != frontp)
		;

	if (str + len - 1 != endp)
		*(endp + 1) = '\0';
	else if (frontp != str && endp == frontp)
		*str = '\0';

	endp = str;
	if (frontp != str) {
		while (*frontp)
			*endp++ = *frontp++;
		*endp = '\0';
	}

	return str;
}

// This is the function to read the config from the device.cfg file
int get_config(char * filename, struct config * configstr) {

	FILE* prop;
	char str1[10], str2[10];
	prop = fopen(filename, "r");
	if (prop == NULL) {
		printf("Config file not found at %s\n",filename);
		return CONFIG_FILE_ERROR;
	}
	char line[256];
	int linenum = 0;
	while (fgets(line, 256, prop) != NULL) {
		char* prop;
		char* value;

		linenum++;
		if (line[0] == '#')
			continue;

		prop = strtok(line, "=");
		prop = trim(prop);
		value = strtok(NULL, "=");
		value = trim(value);
		if (strcmp(prop, "org") == 0)
			strncpy(configstr->org, value, 10);
		else if (strcmp(prop, "type") == 0)
			strncpy(configstr->type, value, 50);
		else if (strcmp(prop, "id") == 0)
			strncpy(configstr->id, value, 50);
		else if (strcmp(prop, "auth-token") == 0)
			strncpy(configstr->authtoken, value, 50);
		else if (strcmp(prop, "auth-method") == 0)
			strncpy(configstr->authmethod, value, 10);
	}

	return SUCCESS;
}

int length(char *str)
{
	int length = 0;

	while (str[length] != '\0')
      length++;

  	return length;
}

//Staggered retry
int retry_connection(GatewayClient *client) 
{
	int retry = 1;
	printf("Attempting to connect\n");

	while(connectGateway(client) != SUCCESS)
	{
		printf("Retry Attempt #%d ", retry);
		int delay = reconnect_delay(retry++);
		printf(" next attempt in %d seconds\n", delay);
		sleep(delay);
	}
}

/* Reconnect delay time 
 * depends on the number of failed attempts
 */
int reconnect_delay(int i) 
{
	if (i < 10) {
		return 3; // first 10 attempts try every 3 seconds
	}
	if (i < 20)
		return 60; // next 10 attempts retry after every 1 minute

	return 600;	// after 20 attempts, retry every 10 minutes
}
