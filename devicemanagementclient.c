/****************************************
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
 *    Hari hara prasad Viswanathan, - initial implementation and API implementation
 *    Hari prasada Reddy P
 *
 *
 ****************************************/


#include <stdio.h>
#include <signal.h>
#include <memory.h>
#include <time.h>
#include "devicemanagementclient.h"

commandCallback cb;

//util functions
void messageCame(MessageData* md);
void generateUUID(char* uuid_str);
int publish(ManagedDevice* client, char* publishTopic, char* data);

static char currentRequestID[40];
volatile int interrupt = 0;

// Handle signal interrupt
void sigHandler(int signo) {
	printf("SigINT received.\n");
	interrupt = 1;
}

/*
* Function used to initialize the IBM Watson IoT client using the config file which is generated when you register your device
*
* @param client - Reference to the ManagedDevice
*
* @param configFilePath - File path to the configuration file
*
* @return int return code
* error codes
* CONFIG_FILE_ERROR -3 - Config file not present or not in right format
*/
int initialize_configfile_dm(ManagedDevice *client, char *configFilePath)
{
	int rc = -1;

	rc = initialize_configfile(&deviceClient, configFilePath);
	
	return rc;
}

/**
* Function used to initialize the IBM Watson IoT client
*
* @param client - Reference to the ManagedDevice
*
* @param org - Your organization ID
*
* @param type - The type of your device
*
* @param id - The ID of your device
*
* @param auth-method - Method of authentication (the only value currently supported is â€œtokenâ€�)
*
* @param auth-token - API key token (required if auth-method is â€œtokenâ€�)
*
* @return int return code
*/
int initialize_dm(ManagedDevice *client, char *orgId, char *deviceType, char *deviceId, char *authmethod, char *authToken)
{
	int rc = -1;
	rc = initialize(&deviceClient, orgId, deviceType, deviceId, authmethod, authToken);
	return rc;
}

/*
* Function used to initialize the IBM Watson IoT client
*
* @param client - Reference to the Iotfclient
*
* @return int return code
*/
int connectiotf_dm(ManagedDevice *client)
{

	int rc = isConnected(&deviceClient);
	if(rc){ 							//if connected return
		printf("Client is connected\n");


		return rc;
	}

	rc = connectiotf(&deviceClient);
	return rc;
}

/*
* Function used to Publish events from the device to the IBM Watson IoT service
*
* @param client - Reference to the ManagedDevice
*
* @param eventType - Type of event to be published e.g status, gps
*
* @param eventFormat - Format of the event e.g json
*
* @param data - Payload of the event
*
* @param QoS - qos for the publish event. Supported values : QOS0, QOS1, QOS2
*
* @return int return code from the publish
*/

int publishEvent_dm(ManagedDevice *client, char *eventType, char *eventFormat, unsigned char* data, enum QoS qos)
{
	int rc = -1;
	rc = publishEvent(&deviceClient, eventType, eventFormat, data, qos);
	return rc;
}

/*
* Function used to set the Command Callback function. This must be set if you to recieve commands.
* 
* @param client reference to the ManagedDevice 
*
* @param handler Function pointer to the commandCallback. Its signature - void (*commandCallback)(char* commandName, char* payload)
* 
*/
void setCommandHandler_dm(ManagedDevice *client, commandCallback handler)
{
	setCommandHandler(&deviceClient,handler );//handler
	cb = handler;
}

/**
 * Register Callback function to managed request response
 * 
 * @param client reference to the ManagedDevice 
 *
 * @param handler Function pointer to the commandCallback. Its signature - void (*commandCallback)(char* Status, char* requestId,            void*       payload)
 *
*/

void setManagedHandler_dm(ManagedDevice *client, commandCallback handler)
{
	cb = handler;
}

/*
* Function used to subscribe to all commands. This function is by default called when in registered mode.
* 
* @param client reference to the ManagedDevice 
*
* @return int return code
*/
int subscribeCommands_dm(ManagedDevice *client)
{
	int rc = -1;

	rc = subscribeCommands(&deviceClient);
	if(rc >=0){
		Client *c;
		c= &(deviceClient.c);
		rc = MQTTSubscribe(c, "iotdm-1/response", QOS0, messageCame);
		rc = MQTTSubscribe(c, "iotdm-1/observe", QOS0, messageCame);
		rc = MQTTSubscribe(c, "iotdm-1/cancel", QOS0, messageCame);
		rc = MQTTSubscribe(c, "iotdm-1/mgmt/initiate/device/reboot", QOS0, messageCame);
		rc = MQTTSubscribe(c, "iotdm-1/mgmt/initiate/device/factory_reset", QOS0, messageCame);
		rc = MQTTSubscribe(c, "iotdm-1/mgmt/initiate/firmware/download", QOS0, messageCame);
		rc = MQTTSubscribe(c, "iotdm-1/mgmt/initiate/firmware/update", QOS0, messageCame);
		rc = MQTTSubscribe(c, "iotdm-1/device/update", QOS0, messageCame);
	}else{
		printf("iotf subscribe faild ");
	}
	return rc;
}

/*
* Function used to Yield for commands.
* 
* @param client reference to the ManagedDevice 
*
* @param time_ms - Time in milliseconds
*
* @return int return code
*/
int yield_dm(ManagedDevice *client, int time_ms)
{
	int rc = 0;
	rc = yield(&deviceClient, time_ms);
	return rc;
}

/*
* Function used to check if the client is connected
* 
* @param client reference to the ManagedDevice 
*
* @return int return code
*/
int isConnected_dm(ManagedDevice *client)
{
	return isConnected(&deviceClient);
}

/*
* Function used to disconnect from the IBM Watson IoT service
* 
* @param client reference to the ManagedDevice 
*
* @return int return code
*/

int disconnect_dm(ManagedDevice *client)
{
	int rc = 0;
	rc = disconnect(&deviceClient);
	return rc;
}

/**
* <p>Send a device manage request to Watson IoT Platform</p>
* 
* <p>A Device uses this request to become a managed device. 
* It should be the first device management request sent by the 
* Device after connecting to the IBM Watson IoT Platform. 
* It would be usual for a device management agent to send this 
* whenever is starts or restarts.</p>
* 
* <p>This method connects the device to Watson IoT Platform connect if its not connected already</p>
* 
* @param client reference to the ManagedDevice
* 
* @param lifetime The length of time in seconds within 
*        which the device must send another Manage device request.
*        if set to 0, the managed device will not become dormant. 
*        When set, the minimum supported setting is 3600 (1 hour).
* 
* @param supportFirmwareActions Tells whether the device supports firmware actions or not.
*        The device must add a firmware handler to handle the firmware requests.
* 
* @param supportDeviceActions Tells whether the device supports Device actions or not.
*        The device must add a Device action handler to handle the reboot and factory reset requests.
*
* @param reqId Function returns the reqId if the publish Manage request is successful.
* 
* @return
*/
void publishManageEvent(ManagedDevice *client, long lifetime, int supportFirmwareActions,
	int supportDeviceActions, char* reqId)
{
		char uuid_str[40];
		generateUUID(uuid_str);
		strcpy(currentRequestID,uuid_str);
		char* strPayload = "{\"d\": {\"metadata\": %s },\"lifetime\": %ld ,\"supports\": {\"deviceActions\": %s,\"firmwareActions\": %s},\"deviceInfo\": {\"serialNumber\":\"%s\",\"manufacturer\":\"%s\",\"model\":\"%s\",\"deviceClass\":\"%s\",\"description\":\"%s\",\"fwVersion\":\"%s\",\"hwVersion\":\"%s\",\"descriptiveLocation\":\"%s\"},\"reqId\": \"%s\"}" ;//cJSON_Print(jsonPayload);
		char payload[1500];
		sprintf(payload,strPayload,client->DeviceData.metadata.metadata,lifetime,"true","true",client->DeviceData.deviceInfo.serialNumber,client->DeviceData.deviceInfo.manufacturer, client->DeviceData.deviceInfo.model,client->DeviceData.deviceInfo.deviceClass,client->DeviceData.deviceInfo.description,client->DeviceData.deviceInfo.fwVersion,client->DeviceData.deviceInfo.hwVersion,client->DeviceData.deviceInfo.descriptiveLocation,uuid_str);
		int rc = -1;
		rc = publish(client, MANAGE, payload);
		if(rc == SUCCESS)
			strcpy(reqId, uuid_str);
}

/**
 * Moves the device from managed state to unmanaged state
 *
 * A device uses this request when it no longer needs to be managed.
 * This means Watson IoT Platform will no longer send new device management requests
 * to this device and device management requests from this device will
 * be rejected apart from a Manage device request
 * 
 * @param client reference to the ManagedDevice
 *
 * @param reqId Function returns the reqId if the Unmanage request is successful.
 */
void publishUnManageEvent(ManagedDevice* client, char* reqId)
{
	char uuid_str[40];
	int rc = -1;
	generateUUID(uuid_str);
	strcpy(currentRequestID,uuid_str);
	char data[70];
	sprintf(data,"{\"reqId\":\"%s\"}",uuid_str);
	rc = publish(client, UNMANAGE, data);
	if(rc == SUCCESS)
		strcpy(reqId, uuid_str);
}

/*
 * Update the location of the device. This method converts the
 * date in the required format. The caller needs to pass the date in string in ISO8601 format.
 * 
 * @param client reference to the ManagedDevice
 *
 * @param latitude	Latitude in decimal degrees using WGS84
 *
 * @param longitude Longitude in decimal degrees using WGS84
 *
 * @param elevation	Elevation in meters using WGS84
 *
 * @param measuredDateTime When the location information is retrieved
 *
 * @param accuracy	Accuracy of the position in meters
 *
 * @param reqId Function returns the reqId if the updateLocation request is successful.
 *
 * @return code indicating whether the update is successful or not
 *        (200 means success, otherwise unsuccessful)

 */
void updateLocation(ManagedDevice* client, double latitude, double longitude, double elevation, char* measuredDateTime, double accuracy, char* reqId)
{
	int rc = -1;

	char uuid_str[40];
	generateUUID(uuid_str);
	strcpy(currentRequestID,uuid_str);

	char data[500];
	sprintf(data,"{\"d\":{\"longitude\":%f,\"latitude\":%f,\"elevation\":%f,\"measuredDateTime\":\"%s\",\"accuracy\":%f},\"reqId\":\"%s\"}", latitude, longitude, elevation, measuredDateTime, accuracy, uuid_str);

	rc = publish(client, UPDATE_LOCATION, data);
	if(rc == SUCCESS)
		strcpy(reqId, uuid_str);
}

/*
 * Update the location of the device. This method converts the
 * date in the required format. The caller needs to pass the date in string in ISO8601 format.
 * 
 * @param client reference to the ManagedDevice
 *
 * @param latitude	Latitude in decimal degrees using WGS84
 *
 * @param longitude Longitude in decimal degrees using WGS84
 *
 * @param elevation	Elevation in meters using WGS84
 *
 * @param measuredDateTime When the location information is retrieved
 *
 * @param accuracy	Accuracy of the position in meters
 *
 * @param reqId Function returns the reqId if the updateLocation request is successful.
 *
 * @return code indicating whether the update is successful or not
 *        (200 means success, otherwise unsuccessful)

 */
void updateLocationEx(ManagedDevice* client, double latitude, double longitude, double elevation, char* measuredDateTime,char* updatedDateTime, double accuracy, char* reqId)
{
	int rc = -1;
	char uuid_str[40];
	generateUUID(uuid_str);
	strcpy(currentRequestID,uuid_str);
		
	char data[500];
	sprintf(data,"{\"d\":{\"longitude\":%f,\"latitude\":%f,\"elevation\":%f,\"measuredDateTime\":\"%s\",\"updatedDateTime\":\"%s\",\"accuracy\":%f},\"reqId\":\"%s\"}", latitude, longitude, elevation, updatedDateTime, updatedDateTime, accuracy, uuid_str);

	rc = publish(client, UPDATE_LOCATION, data);
		
	if(rc == SUCCESS)
		strcpy(reqId, uuid_str);
}

/**
 * Adds the current errorcode to IBM Watson IoT Platform.
 * 
 * @param client reference to the ManagedDevice 
 *
 * @param errorCode The "errorCode" is a current device error code that
 * needs to be added to the Watson IoT Platform.
 *
 * @param reqId Function returns the reqId if the addErrorCode request is successful.
 *
 * @return code indicating whether the update is successful or not
 *        (200 means success, otherwise unsuccessful)
 */
void addErrorCode(ManagedDevice* client, int errNum, char* reqId)
{
	char uuid_str[40];
	generateUUID(uuid_str);
	strcpy(currentRequestID,uuid_str);
	int rc = -1;
	char data[125];
	sprintf(data,"{\"d\":{\"errorCode\":%d},\"reqId\":\"%s\"}", errNum, uuid_str);

	rc = publish(client, CREATE_DIAG_ERRCODES, data);
	if(rc == SUCCESS)
		strcpy(reqId, uuid_str);
}

/**
 * Clear the Error Codes from IBM Watson IoT Platform for this device
 * 
 * @param client reference to the ManagedDevice 
 *
 * @param reqId Function returns the reqId if the clearErrorCodes request is successful.
 *
 * @return code indicating whether the clear operation is successful or not
 *        (200 means success, otherwise unsuccessful)
 */
void clearErrorCodes(ManagedDevice* client, char* reqId)
{
	char uuid_str[40];
	int rc = -1;
	generateUUID(uuid_str);
	strcpy(currentRequestID,uuid_str);

	char data[125];
	sprintf(data,"{\"reqId\":\"%s\"}", uuid_str);

	rc = publish(client, CLEAR_DIAG_ERRCODES, data);
	if(rc == SUCCESS)
		strcpy(reqId, uuid_str);
}

/**
 * The Log message that needs to be added to the Watson IoT Platform.
 * 
 * @param client reference to the ManagedDevice 
 *
 * @param message The Log message that needs to be added to the Watson IoT Platform.
 *
 * @param timestamp The Log timestamp 
 *
 * @param data The optional diagnostic string data
 *
 * @param severity The Log severity
 *
 * @param reqId Function returns the reqId if the addLog request is successful.
 *
 * @return code indicating whether the update is successful or not
 *        (200 means success, otherwise unsuccessful)
 */
void addLog(ManagedDevice* client,char* message, char* data ,int severity, char* reqId)
{
	char uuid_str[40];
	int rc = -1;
	generateUUID(uuid_str);
	strcpy(currentRequestID,uuid_str);
	time_t t = 0;
	char updatedDateTime[50];//"2016-03-01T07:07:56.323Z"
	strftime(updatedDateTime, sizeof(updatedDateTime), "%Y-%m-%dT%TZ", localtime(&t));
	char payload[125];
	sprintf(payload,"{\"d\":{\"message\":\"%s\",\"timestamp\":\"%s\",\"data\":\"%s\",\"severity\":%d},\"reqId\":\"%s\"}",message,updatedDateTime,data,severity, uuid_str);

	rc = publish(client, ADD_DIAG_LOG, payload );
	if(rc == SUCCESS)
		strcpy(reqId, uuid_str);
}

/**
 * Clear the Logs from IBM Watson IoT Platform for this device
 * 
 * @param client reference to the ManagedDevice 
 *
 * @param reqId Function returns the reqId if the clearLogs request is successful.
 *
 * @return code indicating whether the clear operation is successful or not
 *        (200 means success, otherwise unsuccessful)
 */
void clearLogs(ManagedDevice* client, char* reqId){
	char uuid_str[40];
	int rc = -1;
	generateUUID(uuid_str);
	strcpy(currentRequestID,uuid_str);

	char data[125];
	sprintf(data,"{\"reqId\":\"%s\"}", uuid_str);

	rc = publish(client, CLEAR_DIAG_LOG, data);
	if(rc == SUCCESS)
		strcpy(reqId, uuid_str);
}

// Utility function to publish the message to Watson IoT
int publish(ManagedDevice* client, char* publishTopic, char* data)
{
	int rc = -1;
	MQTTMessage pub;
	printf("Topic ( %s) payload (\n %s\n ) ", publishTopic,data);
	pub.qos = 1;
	pub.retained = '0';
	pub.payload = data;
	pub.payloadlen = strlen(data);
	signal(SIGINT, sigHandler);
	signal(SIGTERM, sigHandler);
	interrupt =0;
	while(!interrupt)
		{
			rc = MQTTPublish(&deviceClient.c, publishTopic , &pub);
			if(rc == SUCCESS) {
				rc = yield(&deviceClient, 100);
			}
			if(!interrupt)
				sleep(2);
		}
	return rc;
}

// Utility function to generate Unique Identifier
void generateUUID(char* uuid_str)
{
	char GUID[40];
	int t = 0;
	char *szTemp = "xxxxxxxx-xxxxy-4xxxx-yxxxy-xxxxxxxxxxxx";
	char *szHex = "0123456789ABCDEF-";
	int nLen = strlen (szTemp);

	for (t=0; t<nLen+1; t++)
	{
	    int r =( rand ())% 16;
	    char c = ' ';

	    switch (szTemp[t])
	    {
	        case 'x' : { c = szHex [r]; } break;
	        case 'y' : { c = szHex [((r & 0x03) | 0x08)]; } break;
	        case '-' : { c = '-'; } break;
	        case '4' : { c = '4'; } break;
	    }

	    GUID[t] = ( t < nLen ) ? c : 0x00;
	}
	strcpy(uuid_str , GUID);
}

//Handler for all commands. Invoke the callback.
void messageCame(MessageData* md)
{
	if(cb != 0) {
		MQTTMessage* message = md->message;
		void *payload = message->payload;
		char *pl = (char*) malloc(sizeof(char)*message->payloadlen+1);
		strcpy(pl,message->payload);
		char *reqID;
		char *status;

		reqID = strtok(pl, ",");
		status= strtok(NULL, ",");

		reqID = strtok(reqID, ":\"");
		reqID = strtok(NULL, ":\"");
		reqID = strtok(NULL, ":\"");
		
		status= strtok(status, "}");
		status= strtok(status, ":");
		status= strtok(NULL, ":");

		//printf("reqId:%s %d \tcurrent:%s %d\n",reqID, strlen(reqID),currentRequestID, strlen(currentRequestID));
		if(!strcmp(currentRequestID,reqID))
		{
			interrupt = 1;
			(*cb)(status, reqID, payload);
		}
		else
		{
			printf("different response came for ID: %s\n",reqID);
		}
		free(pl);
	}
}
