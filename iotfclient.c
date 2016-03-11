/*******************************************************************************
 * Copyright (c) 2015, 2016 IBM Corp.
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

#include <stdio.h>
#include <signal.h>
#include <memory.h>

#include <sys/time.h>
#include "iotfclient.h"

//Command Callback
commandCallback cb;

//util functions
char *trim(char *str);
int get_config(char * filename, struct config * configstr);
void messageArrived(MessageData* md);
int length(char *str);
int retry_connection(Iotfclient *client);
int reconnect_delay(int i);


/**
* Function used to initialize the IBM Watson IoT client using the config file which is generated when you register your device
* @param configFilePath - File path to the configuration file 
*
* @return int return code
* error codes
* CONFIG_FILE_ERROR -3 - Config file not present or not in right format
*/
int initialize_configfile(Iotfclient *client, char *configFilePath)
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
* Function used to initialize the IBM Watson IoT client
* @param org - Your organization ID
* @param type - The type of your device
* @param id - The ID of your device
* @param auth-method - Method of authentication (the only value currently supported is “token”)
* @param auth-token - API key token (required if auth-method is “token”)
*
* @return int return code
*/
int initialize(Iotfclient *client, char *orgId, char *deviceType, char *deviceId, char *authmethod, char *authToken)
{

	struct config configstr = {"", "", "", "", ""};

	if(orgId==NULL || deviceType==NULL || deviceId==NULL) {
		return MISSING_INPUT_PARAM;
	}

	strncpy(configstr.org, orgId, 15);
	strncpy(configstr.type, deviceType, 50);
	strncpy(configstr.id, deviceId, 50);

	if((strcmp(orgId,"quickstart") != 0)) {
		if(authmethod == NULL || authToken == NULL) {
			return MISSING_INPUT_PARAM;
		}
		strncpy(configstr.authmethod, authmethod, 10);
		strncpy(configstr.authtoken, authToken, 50);
	}

	client->config = configstr;

	return SUCCESS;
}

/**
* Function used to initialize the IBM Watson IoT client
* @param client - Reference to the Iotfclient
*
* @return int return code
*/
int connectiotf(Iotfclient *client)
{

	int rc = 0;
	client->isQuickstart = 0;
	if(strcmp(client->config.org,"quickstart") == 0){
		client->isQuickstart = 1 ;
	}

	const char* messagingUrl = ".messaging.internetofthings.ibmcloud.com";

	char hostname[strlen(client->config.org) + strlen(messagingUrl) + 1];
	
	sprintf(hostname, "%s%s", client->config.org, messagingUrl);

    //TODO : change to 8883 if registered, add support when available in MQTTClient
    int port = 1883;

    char clientId[strlen(client->config.org) + strlen(client->config.type) + strlen(client->config.id) + 5];
    sprintf(clientId, "d:%s:%s:%s", client->config.org, client->config.type, client->config.id);

	NewNetwork(&client->n);
	ConnectNetwork(&client->n, hostname, port);
	MQTTClient(&client->c, &client->n, 1000, client->buf, 1000, client->readbuf, 1000);
 
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;       
	data.willFlag = 0;
	data.MQTTVersion = 3;
	data.clientID.cstring = clientId;

	if(!client->isQuickstart) {
		printf("Connecting to registered service with org %s\n", client->config.org);
		data.username.cstring = "use-token-auth";
		data.password.cstring = client->config.authtoken;
	}

	data.keepAliveInterval = 60;
	data.cleansession = 1;
	
	rc = MQTTConnect(&client->c, &data);

	if(!client->isQuickstart) {
		//Subscibe to all commands
		subscribeCommands(client);
	}

	return rc;
}

/**
* Function used to Publish events from the device to the IBM Watson IoT service
* @param eventType - Type of event to be published e.g status, gps
* @param eventFormat - Format of the event e.g json
* @param data - Payload of the event
* @param QoS - qos for the publish event. Supported values : QOS0, QOS1, QOS2
*
* @return int return code from the publish
*/

int publishEvent(Iotfclient *client, char *eventType, char *eventFormat, unsigned char* data, enum QoS qos)
{
	int rc = -1;

	char publishTopic[strlen(eventType) + strlen(eventFormat) + 16];

	sprintf(publishTopic, "iot-2/evt/%s/fmt/%s", eventType, eventFormat);

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
void setCommandHandler(Iotfclient *client, commandCallback handler)
{
	cb = handler;
}

/**
* Function used to subscribe to all commands. This function is by default called when in registered mode.
*
* @return int return code
*/
int subscribeCommands(Iotfclient *client) 
{
	int rc = -1;

	rc = MQTTSubscribe(&client->c, "iot-2/cmd/+/fmt/+", QOS0, messageArrived);

	return rc;
}

/**
* Function used to Yield for commands.
* @param time_ms - Time in milliseconds
* @return int return code
*/
int yield(Iotfclient *client, int time_ms)
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
int isConnected(Iotfclient *client)
{
	return client->c.isconnected;
}

/**
* Function used to disconnect from the IBM Watson IoT service
*
* @return int return code
*/

int disconnect(Iotfclient *client)
{
	int rc = 0;
	rc = MQTTDisconnect(&client->c);
	client->n.disconnect(&client->n);

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

		strtok(topic, "/");
		strtok(NULL, "/");

		char *commandName = strtok(NULL, "/");
		strtok(NULL, "/");
		char *format = strtok(NULL, "/");

		free(topic);

		(*cb)(commandName, format, payload);
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
int retry_connection(Iotfclient *client) 
{
	int retry = 1;
	printf("Attempting to connect\n");

	while(connectiotf(client) != SUCCESS)
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
