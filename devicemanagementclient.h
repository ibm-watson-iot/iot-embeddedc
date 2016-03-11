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
 *    Hari hara prasad Viswanathan, - initial implementation and API implementation
 *    Hari prasada Reddy P
 *
 *
 *******************************************************************************/

#ifndef DEVICEMANAGEMENTCLIENT_H_
#define DEVICEMANAGEMENTCLIENT_H_

#include "lib/MQTTClient.h"
#include "iotfclient.h"
#include <ctype.h>
#include <stdbool.h>

//Macros for the device management requests
#define MANAGE "iotdevice-1/mgmt/manage"
#define UNMANAGE "iotdevice-1/mgmt/unmanage"
#define UPDATE_LOCATION "iotdevice-1/device/update/location"
#define CREATE_DIAG_ERRCODES "iotdevice-1/add/diag/errorCodes"
#define CLEAR_DIAG_ERRCODES "iotdevice-1/clear/diag/errorCodes"
#define ADD_DIAG_LOG "iotdevice-1/add/diag/log"
#define CLEAR_DIAG_LOG "iotdevice-1/clear/diag/log"
#define NOTIFY "iotdevice-1/notify"
#define RESPONSE "iotdevice-1/response"

//structure for device information
struct DeviceInfo{
	char serialNumber[20];
	char manufacturer[20];
	char model[20];
	char deviceClass[20];
	char description[30];
	char fwVersion[10];
	char hwVersion[10];
	char descriptiveLocation[20];
};

//structure for device location
struct DeviceLocation{
	double latitude;
	double longitude;
	double elevation;
	time_t measuredDateTime;
	double accuracy;
};

//structure for device actions
struct DeviceAction{
	int status;
	char message[50];
	char typeId[10];
	char deviceId[10];
};

//structure for device firmware attributes
struct DeviceFirmware{
	char version[10];
	char name[10];
	char url[10];
	char verifier[10];
	int state;
	int updateStatus;
	char deviceId[10];
	char typeId[10];
};

//structure for metadata of device
struct DeviceMetadata{
	char metadata[10];
};

//structure for device management
struct DeviceMgmt{
	struct DeviceFirmware firmware;
};

//structure for device data
struct deviceData{
	struct DeviceInfo deviceInfo;
	struct DeviceLocation deviceLocation;
	struct DeviceMgmt mgmt;
	struct DeviceMetadata metadata;
	struct DeviceAction deviceAction;
};

struct managedDevice{
		bool supportDeviceActions ;
		bool supportFirmwareActions ;
		bool bManaged ;
		char responseSubscription[50];
		struct deviceData DeviceData;
};
typedef struct managedDevice ManagedDevice;

struct iotfclient deviceClient;
/**
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
int initialize_configfile_dm(ManagedDevice *client, char *configFilePath);

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
int initialize_dm(ManagedDevice *client, char *orgId, char *deviceType, char *deviceId, char *authmethod, char *authToken);

/**
* Function used to connect the device to IBM Watson IoT client
*
* @param client - Reference to the ManagedDevice
*
* @return int return code
*/

int connectiotf_dm(ManagedDevice* client);
/**
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
int publishEvent_dm(ManagedDevice *client, char *eventType, char *eventFormat, unsigned char* data, enum QoS qos);

/**
* Function used to set the Command Callback function. This must be set if you want to receive commands.
*
* @param client - Reference to the ManagedDevice
*
* @param cb - A Function pointer to the commandCallback. Its signature - void (*commandCallback)(char* commandName, char* format, void*     payload)
*
*/
void setCommandHandler_dm(ManagedDevice *client, commandCallback cb);

/**
* Function used to subscribe to all commands. This function is by default called when in registered mode.
*
* @param client - Reference to the ManagedDevice
*
* @return int return code
*/
int subscribeCommands_dm(ManagedDevice *client);

/**
* Function used to check if the client is connected
*
* @param client - Reference to the ManagedDevice
*
* @return int return code
*/
int isConnected_dm(ManagedDevice *client);

/**
* Function used to Yield for commands.
*
* @param client - Reference to the ManagedDevice
*
* @param time_ms - Time in milliseconds
*
* @return int return code
*/
int yield_dm(ManagedDevice *client, int time_ms);

/**
* Function used to disconnect from the IBM Watson IoT service
*
* @param client - Reference to the ManagedDevice
*
* @return int return code
*/
int disconnect_dm(ManagedDevice *client);

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
	int supportDeviceActions, char* reqId);
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
 *
 */
void publishUnManageEvent(ManagedDevice* client, char* reqId);

/**
 * Update the location.
 *
 * @param client reference to the ManagedDevice
 * 
 * @param latitude	Latitude in decimal degrees using WGS84
 *
 * @param longitude Longitude in decimal degrees using WGS84
 *
 * @param elevation	Elevation in meters using WGS84 
 *
 * @param measuredDateTime	Date of location measurement in ISO8601 format
 *
 * @param accuracy	Accuracy of the position in meters
 *
 * @param reqId Function returns the reqId if the updatelocation request is successful.
 *
 * @return code indicating whether the update is successful or not
 *        (200 means success, otherwise unsuccessful)
 */
void updateLocation(ManagedDevice* client, double latitude, double longitude, double elevation, char* measuredDateTime, double accuracy, char* reqId) ;

/**
 * Update the location.
 * 
 * @param client reference to the ManagedDevice 
 *
 * @param latitude	Latitude in decimal degrees using WGS84
 *
 * @param longitude Longitude in decimal degrees using WGS84
 *
 * @param elevation	Elevation in meters using WGS84 
 *
 * @param measuredDateTime	Date of location measurement in ISO8601 format
 *
 * @param updatedDateTime	Date of the update to the device information in ISO8601 format
 *
 * @param accuracy	Accuracy of the position in meters
 *
 * @param reqId Function returns the reqId if the UpdateLocation request is successful.
 *
 * @return code indicating whether the update is successful or not
 *        (200 means success, otherwise unsuccessful)
 */
void updateLocationEx(ManagedDevice* client, double latitude, double longitude, double elevation, char* measuredDateTime,char* updatedDateTime, double accuracy, char* reqId);
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
void addErrorCode(ManagedDevice* client, int errNum, char* reqId);
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
void clearErrorCodes(ManagedDevice* client, char* reqId);
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
void addLog(ManagedDevice* client,char* message, char* data ,int severity, char* reqId);
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
void clearLogs(ManagedDevice* client, char* reqId);

/**
 * Register Callback function to managed request response
 * 
 * @param client reference to the ManagedDevice 
 *
 * @param cb - A Function pointer to the commandCallback. Its signature - void (*commandCallback)(char* Status, char* requestId,            void*       payload)
 *
*/
void setManagedHandler_dm(ManagedDevice *client, commandCallback handler);

#endif /* DEVICEMANAGEMENTCLIENT_H_ */
