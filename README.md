Embedded C Client Library - Introduction
========================================

Embedded C client for interacting with the IBM Watson Internet of Things
Platform.

Dependencies
------------

1.  [Embedded C MQTT Client]

  [Embedded C MQTT Client]: http://www.eclipse.org/paho/clients/c/embedded/
  
Build Dependencies
------------------

1.  [CMake 2.8+]

  [CMake 2.8+]: http://cmake.org

Build & Install
--------------

~~~~~~~~~~~~~{.sh}
mkdir build
cd build
cmake ..
make
~~~~~~~~~~~~~

One built you can install the libraries by running:

```bash
sudo make install
```

Embedded C Client Library - Devices
===================================

*iotfclient* is client for the IBM Watson Internet of Things
Platform service.
You can use this client to connect to the service, publish events and
subscribe to commands.

Initialize
----------

There are 2 ways to initialize the *iotfclient*.

### Passing as parameters

The function *initialize* takes the following details to connect to the
IBM Watson Internet of Things Platform service

-   client - Pointer to the *iotfclient*
-   org - Your organization ID
-   type - The type of your device
-   id - The ID of your device
-   authmethod - Method of authentication (the only value currently
    supported is “token”)
-   authtoken - API key token (required if auth-method is “token”)

``` {.sourceCode .c}
#include "iotfclient.h"
   ....
   ....
   Iotfclient client;
   //quickstart
   rc = initialize(&client,"quickstart","iotsample","001122334455",NULL,NULL);
   //registered
   rc = initialize(&client,"orgid","type","id","token","authtoken");
   ....
```

### Using a configuration file

The function *initialize\_configfile* takes the configuration file path
as a parameter.

``` {.sourceCode .c}
#include "iotfclient.h"
   ....
   ....
   char *filePath = "./device.cfg";
   Iotfclient client;
   rc = initialize_configfile(&client, filePath);
   ....
```

The configuration file must be in the format of

``` {.sourceCode .}
org=$orgId
type=$myDeviceType
id=$myDeviceId
auth-method=token
auth-token=$token
```

##### Return codes

Following are the return codes in the *initialize* function

* CONFIG_FILE_ERROR   -3
* MISSING_INPUT_PARAM   -4


Connect
-------

After initializing the *iotfclient*, you can connect to the IBM Watson Internet of Things
Platform by calling the *connectiotf* function

``` {.sourceCode .c}
#include "iotfclient.h"
   ....
   ....
   Iotfclient client;
   char *configFilePath = "./device.cfg";

   rc = initialize_configfile(&client, configFilePath);

   if(rc != SUCCESS){
       printf("initialize failed and returned rc = %d.\n Quitting..", rc);
       return 0;
   }

   rc = connectiotf(&client);

   if(rc != SUCCESS){
       printf("Connection failed and returned rc = %d.\n Quitting..", rc);
       return 0;
   }
   ....
```

##### Return Codes

The IoTF *connectiotf* function return codes

* MQTTCLIENT_SUCCESS   0
* MQTTCLIENT_FAILURE   -1
* MQTTCLIENT_DISCONNECTED   -3
* MQTTCLIENT_MAX_MESSAGES_INFLIGHT   -4
* MQTTCLIENT_BAD_UTF8_STRING   -5
* MQTTCLIENT_BAD_QOS   -9
* MQTTCLIENT_NOT_AUTHORIZED   5

Handling commands
-----------------

When the device client connects, it automatically subscribes to any
command for this device. To process specific commands you need to
register a command callback function by calling the function
*setCommandHandler*. The commands are returned as

-   commandName - name of the command invoked
-   format - e.g json, xml
-   payload

``` {.sourceCode .c}
#include "iotfclient.h"

void myCallback (char* commandName, char* format, void* payload)
{
   printf("The command received :: %s\n", commandName);
   printf("format : %s\n", format);
   printf("Payload is : %s\n", (char *)payload);
}
 ...
 ...
 char *filePath = "./device.cfg";
 rc = connectiotfConfig(filePath);
 setCommandHandler(myCallback);

 yield(1000);
 ....
```

**Note** : *yield* function must be called periodically to receive commands.

Publishing events
------------------

Events can be published by using

-   eventType - Type of event to be published e.g status, gps
-   eventFormat - Format of the event e.g json
-   data - Payload of the event
-   QoS - qos for the publish event. Supported values : QOS0, QOS1, QOS2

``` {.sourceCode .c}
#include "iotfclient.h"
 ....
 rc = connectiotf (org, type, id , authmethod, authtoken);
 char *payload = {\"d\" : {\"temp\" : 34 }};

 rc= publishEvent("status","json", "{\"d\" : {\"temp\" : 34 }}", QOS0); 
 ....
```

Disconnect Client
------------------

Disconnects the client and releases the connections

``` {.sourceCode .c}
#include "iotfclient.h"
 ....
 rc = connectiotf (org, type, id , authmethod, authtoken);
 char *payload = {\"d\" : {\"temp\" : 34 }};

 rc= publishEvent("status","json", payload , QOS0);
 ...
 rc = disconnect();
 ....
```


Embedded C Client Library - Gateways
=====================================

*gatewayclient* is the gateway client for the IBM Watson Internet of Things Platform.
You can use this client to connect to the platform, publish gateway events, publish device events on behalf of the devices, subscribe to both gateway and device commands. 

Initialize
----------

There are 2 ways to initialize the *gatewayclient*.

### Passing as parameters

The function *initializeGateway* takes the following details to connect to the
IBM Watson Internet of Things Platform service

-   client - Pointer to the *gatewayclient*
-   org - Your organization ID
-   type - The type of your gateway
-   id - The ID of your gateway
-   authmethod - Method of authentication (the only value currently
    supported is “token”)
-   authtoken - Gateway authentication token (required if auth-method is “token”)

``` {.sourceCode .c}
#include "gatewayclient.h"
   ....
   ....
   GatewayClient client;
   rc = initializeGateway(&client, "org", "gatewayTyoe","gateway01" "token", "sdJh&usdhk#kjhsd");
   if(rc != SUCCESS){
    printf("initialize failed and returned rc = %d.\n Quitting..", rc);
    return 0;
  }

  rc = connectGateway(&client);
   ....
```

### Using a configuration file

The function *initializeGateway_configfile* takes the configuration file path
as a parameter.

``` {.sourceCode .c}
#include "gatewayclient.h"
   ....
   ....
   GatewayClient client;
   char *configFilePath = "./gateway.cfg";
   rc = initializeGateway_configfile(&client, configFilePath);
   
   if(rc != SUCCESS){
    printf("initialize failed and returned rc = %d.\n Quitting..", rc);
    return 0;
  }

  rc = connectGateway(&client);
   ....
```

The configuration file must be in the format of

``` {.sourceCode .}
org=$orgId
type=$myGatewayType
id=$myGatewayId
auth-method=token
auth-token=$token
```

##### Return codes

Following are the return codes in the *initializeGateway* and *initializeGateway_configfile* function

* CONFIG_FILE_ERROR   -3
* MISSING_INPUT_PARAM   -4
* QUICKSTART_NOT_SUPPORTED  -5 


Connect
-------

After initializing the *gatewayclient*, you can connect to IBM Watson Internet of
Things Platform by calling the *connectGateway* function

``` {.sourceCode .c}
#include "gatewayclient.h"
   ....
   ....
   char *configFilePath = "./gateway.cfg";

  rc = initializeGateway_configfile(&client, configFilePath);

  if(rc != SUCCESS){
    printf("initialize failed and returned rc = %d.\n Quitting..", rc);
    return 0;
  }

  rc = connectGateway(&client);

  if(rc != SUCCESS){
    printf("Connection failed and returned rc = %d.\n Quitting..", rc);
    return 0;
  }
   ....
```

##### Return Codes

The IoTF *connectiotf* function return codes

* MQTTCLIENT_SUCCESS   0
* MQTTCLIENT_FAILURE   -1
* MQTTCLIENT_DISCONNECTED   -3
* MQTTCLIENT_MAX_MESSAGES_INFLIGHT   -4
* MQTTCLIENT_BAD_UTF8_STRING   -5
* MQTTCLIENT_BAD_QOS   -9
* MQTTCLIENT_NOT_AUTHORIZED   5

Handling commands
-----------------

When the gateway client connects, it automatically subscribes to all
commands for this gateway. 
For subscribing for device commands you need to use *subscribeToDeviceCommands*. You need to provide the device Type , deivce Id, command name, the command format and QOS.

``` {.sourceCode .c}
#include "gatewayclient.h"
   ....
   ....
   char *configFilePath = "./gateway.cfg";

  rc = initializeGateway_configfile(&client, configFilePath);

  if(rc != SUCCESS){
    printf("initialize failed and returned rc = %d.\n Quitting..", rc);
    return 0;
  }

  rc = connectGateway(&client);

  if(rc != SUCCESS){
    printf("Connection failed and returned rc = %d.\n Quitting..", rc);
    return 0;
  }
  
  setGatewayCommandHandler(&client, myCallback);
  // providing "+" will subscribe to all the command of all formats.
  subscribeToDeviceCommands(&client, "pitype", "pi2", "+", "+", 0);
   ....
```

##### Process Commands

To process specific commands you need to register a command callback function by calling the function
*setGatewayCommandHandler*. The commands are returned as

-   type - Type of the Gateway/Device
-   id - ID of the Gateway/Device
-   commandName - name of the command invoked
-   format - e.g json, xml
-   payload
-   payloadlen - Length of the payload

``` {.sourceCode .c}
#include "gatewayclient.h"

void myCallback (char* type, char* id, char* commandName, char *format, void* payload, size_t payloadlen)
{
  printf("------------------------------------\n" );
  printf("Type is : %s\n", type);
  printf("Id is : %s\n", id);
  printf("The command received :: %s\n", commandName);
  printf("format : %s\n", format);
  printf("Payload is : %.*s\n", (int)payloadlen, (char *)payload);
  printf("------------------------------------\n" );
}
 ...
 ...
    char *configFilePath = "./gateway.cfg";

  rc = initializeGateway_configfile(&client, configFilePath);

  if(rc != SUCCESS){
    printf("initialize failed and returned rc = %d.\n Quitting..", rc);
    return 0;
  }

  rc = connectGateway(&client);

  if(rc != SUCCESS){
    printf("Connection failed and returned rc = %d.\n Quitting..", rc);
    return 0;
  }

  setGatewayCommandHandler(&client, myCallback);
  // providing "+" will subscribe to all the command of all formats.
  subscribeToDeviceCommands(&client, "raspi", "pi2", "+", "+", 0);

    gatewayYield(1000);
 ....
```

**Note** : *gatewayYield* function must be called periodically to receive commands.

Publishing events
------------------
A gateway can publish events from itself and on behalf of any device connected via the gateway. 
Events can be published by using
-   eventType - Type of event to be published e.g status, gps
-   eventFormat - Format of the event e.g json
-   data - Payload of the event
-   QoS - qos for the publish event. Supported values : QOS0, QOS1, QOS2

##### Publish Gateway Events
    
``` {.sourceCode .c}
    rc = connectGateway(&client);

  if(rc != SUCCESS){
    printf("Connection failed and returned rc = %d.\n Quitting..", rc);
    return 0;
  }
  rc= publishGatewayEvent(&client, "raspi","device02","status","json", "{\"d\" : {\"temp\" : 34 }}", QOS0);
 ....
```

##### Publish Device Events on behalf of a device
    
``` {.sourceCode .c}
    rc = connectGateway(&client);

  if(rc != SUCCESS){
    printf("Connection failed and returned rc = %d.\n Quitting..", rc);
    return 0;
  }
  rc= publishDeviceEvent(&client, "raspi","device02","status","json", "{\"d\" : {\"temp\" : 34 }}", QOS0);
 ....
```

Disconnect Client
------------------

Disconnects the client and releases the connections

``` {.sourceCode .c}
    rc = connectGateway(&client);

  if(rc != SUCCESS){
    printf("Connection failed and returned rc = %d.\n Quitting..", rc);
    return 0;
  }
  rc= publishDeviceEvent(&client, "raspi","device02","status","json", "{\"d\" : {\"temp\" : 34 }}", QOS0);
  disconnectGateway(&client);
```

======================================
Embedded C Library - Managed Device
======================================

Introduction
-------------

This client library describes how to use devices with the Embedded C WIoTP client library.

This section contains information on how devices can connect to the Internet of Things Platform Device Management service using c and perform device management operations like  location update, add logs and diagnostics update.



Device Management
-------------------------------------------------------------------------------
The device management <a href="https://docs.internetofthings.ibmcloud.com/devices/device_mgmt/index.html">device management</a> feature enhances the IBM Watson Internet of Things Platform service with new capabilities for managing devices. Device management makes a distinction between managed and unmanaged devices:

* **Managed Devices** are defined as devices which have a management agent installed. The management agent sends and receives device metadata and responds to device management commands from the IBM Watson Internet of Things Platform.
* **Unmanaged Devices** are any devices which do not have a device management agent. All devices begin their lifecycle as unmanaged devices, and can transition to managed devices by sending a message from a device management agent to the IBM Watson Internet of Things Platform.


---------------------------------------------------------------------------
Connecting to the Internet of Things Platform Device Management Service
---------------------------------------------------------------------------
Initialize
----------

There are 2 ways to initialize the *iotfclient*.

### Passing as parameters

The function *initialize* takes the following details to connect to the
IBM Watson Internet of Things Platform service

-   client - Pointer to the *iotfclient*
-   org - Your organization ID
-   type - The type of your device
-   id - The ID of your device
-   authmethod - Method of authentication (the only value currently
    supported is “token”)
-   authtoken - API key token (required if auth-method is “token”)

``` {.sourceCode .c}
#include "devicemanagementclient.h"
   ....
   ....
   ManagedDevice client;
   //quickstart
   rc = initialize_dm(&client,"quickstart","iotsample","001122334455",NULL,NULL);
   //registered
   rc = initialize_dm(&client,"orgid","type","id","token","authtoken");
   ....
```

### Using a configuration file

The function *initialize\_configfile* takes the configuration file path
as a parameter.

``` {.sourceCode .c}
#include "devicemanagementclient.h"
   ....
   ....
   char *filePath = "./device.cfg";
   ManagedDevice client;
   rc = initialize_configfile_dm(&client, filePath);
   ....
```

The configuration file must be in the format of

``` {.sourceCode .}
org=$orgId
type=$myDeviceType
id=$myDeviceId
auth-method=token
auth-token=$token
```

##### Return codes

Following are the return codes in the *initialize* function

* CONFIG_FILE_ERROR   -3
* MISSING_INPUT_PARAM   -4


Connect
-------

After initializing the *iotfclient*, you can connect to the IBM Watson Internet of Things
Platform by calling the *connectiotf_dm* function

``` {.sourceCode .c}
#include "devicemanagementclient.h"
   ....
   ....
   ManagedDevice client;
   char *configFilePath = "./device.cfg";

   rc = initialize_configfile_dm(&client, configFilePath);

   if(rc != SUCCESS){
       printf("initialize failed and returned rc = %d.\n Quitting..", rc);
       return 0;
   }

   rc = connectiotf_dm(&client);

   if(rc != SUCCESS){
       printf("Connection failed and returned rc = %d.\n Quitting..", rc);
       return 0;
   }
   ....
```

##### Return Codes

The IoTF *connectiotf_dm* function return codes

* MQTTCLIENT_SUCCESS   0
* MQTTCLIENT_FAILURE   -1
* MQTTCLIENT_DISCONNECTED   -3
* MQTTCLIENT_MAX_MESSAGES_INFLIGHT   -4
* MQTTCLIENT_BAD_UTF8_STRING   -5
* MQTTCLIENT_BAD_QOS   -9
* MQTTCLIENT_NOT_AUTHORIZED   5

Create DeviceData
------------------------------------------------------------------------
The  <a href="https://docs.internetofthings.ibmcloud.com/reference/device_model.html">device model </a> describes the metadata and management characteristics of a device. The device database in the IBM Watson Internet of Things Platform is the master source of device information. Applications and managed devices are able to send updates to the database such as a location or the progress of a firmware update. Once these updates are received by the IBM Watson Internet of Things Platform, the device database is updated, making the information available to applications.

The device model in the WIoTP client library is represented as DeviceData and to create a DeviceData one needs to create the following objects,

* DeviceInfo (Optional)
* DeviceLocation (Optional, required only if the device wants to be notified about the location set by the application through Watson IoT Platform API)
* DeviceFirmware (Optional)
* DeviceMetadata (optional)

The following code snippet shows how to create the object ManagedDevice along with DeviceMetadata with sample data:

``` {.sourceCode .c}
ManagedDevice client;
strcpy(client.DeviceData.deviceInfo.serialNumber, "10087" );
strcpy(client.DeviceData.deviceInfo.manufacturer , "IBM");
strcpy(client.DeviceData.deviceInfo.model , "7865");
strcpy(client.DeviceData.deviceInfo.deviceClass , "A");
strcpy(client.DeviceData.deviceInfo.description , "My Ras");
strcpy(client.DeviceData.deviceInfo.fwVersion , "1.0.0");
strcpy(client.DeviceData.deviceInfo.hwVersion , "1.0");
strcpy(client.DeviceData.deviceInfo.descriptiveLocation , "EGL C");
strcpy(client.DeviceData.metadata.metadata ,"{}");    
```
Note : meta data value must be a json string, for example :"{\"key\":\"string value \"}"

Register Callback function
------------------------------------------------------------------
 To process the response of device management request we need to register a  callback function by calling the function
*setManagedHandler_dm*. The commands are returned as

-   Status - status of response
-   requestId -  ID of the request to which the response is.
-   payload

``` {.sourceCode .c}
#include "devicemanagementclient.h"

void managedCallBack (char* Status, char* requestId, void* payload)
{
	printf("\n------------------------------------\n" );
	printf("Status :: %s\n", Status);
	printf("requestId : %s\n", requestId);
	printf("Payload is : %s\n", (char *)payload);
	printf("------------------------------------\n" );
}
 ...
 ...
 ManagedDevice client;
 char *filePath = "./device.cfg";
 rc = connectiotfConfig_dm(filePath);
 setCommandHandler_dm(myCallback);
 setManagedHandler_dm(&client,managedCallBack );

 ....
```
Following are the status code for the device Management response,
<ul>
    <li>200: The operation was successful.</li>
    <li>400: The input message does not match the expected format, or one of the values is out of the valid range.</li>
    <li>404: The topic name is incorrect, or the device is not in the database.</li>
    <li>409: A conflict occurred during the device database update. To resolve this, simplify the operation is necessary.</li>
</ul>



Subscribe
-----------------------------------------------------------------
In order to get the response for the each manage device request we need to make an subscription to all the commands by calling
*subscribeCommands_dm* function
``` {.sourceCode .c}
#include "devicemanagementclient.h"
 ...
 ...
 ManagedDevice client;
 char *filePath = "./device.cfg";
 rc = connectiotfConfig_dm(filePath);
 setCommandHandler_dm(myCallback);
 setManagedHandler_dm(&client,managedCallBack );
 subscribeCommands_dm(&client);
 ....
```



Manage
------------------------------------------------------------------
The device can invoke publishManageEvent() function to participate in device management activities of the IBM Watson Internet of Things Platform

``` {.sourceCode .c}
	publishManageEvent(&client,4000,1,1, reqId);
```
As shown, this function accepts following 5 parameters,
* *ManagedDevice struct Instance* which has all the device info filled
* *lifetime* The length of time in seconds within which the device must send another **Manage device** request in order to avoid being reverted to an unmanaged device and marked as dormant. If set to 0, the managed device will not become dormant. When set, the minimum supported setting is 3600 (1 hour).
* *supportFirmwareActions* Tells whether the device supports firmware actions or not. The device must add a firmware handler to handle the firmware requests.
* *supportDeviceActions* Tells whether the device supports Device actions or not. The device must add a Device action handler to handle the reboot and factory reset
 requests.
* *request ID* out value of Request Id for the current request.


Refer to the  <a href="https://docs.internetofthings.ibmcloud.com/devices/device_mgmt/index.html#/manage-device#manage-device">documentation</a> for more information about the manage operation.

Unmanage
-----------------------------------------------------

A device can invoke sendUnmanageRequest() function when it no longer needs to be managed. The IBM Watson Internet of Things Platform will no longer send new device management requests to this device and all device management requests from this device will be rejected other than a **Manage device** request.

``` {.sourceCode .c}
	publishUnManageEvent(&client, reqId);
```
As shown, this function accepts following 2 parameters,
* *ManagedDevice struct Instance* which has all the device info filled
* *request ID* out value of Request Id for the current request.

Refer to the  <a href="https://docs.internetofthings.ibmcloud.com/devices/device_mgmt/index.html#/unmanage-device#unmanage-device">documentation</a> for more information about the Unmanage operation.

Location Update
-----------------------------------------------------

Devices that can determine their location can choose to notify the IBM Watson Internet of Things Platform about location changes. The Device can invoke one of the overloaded updateLocation() function to update the location of the device.

``` {.sourceCode .c}
  updateLocation(&client, 77.5667,12.9667, 0,updatedDateTime, 0, reqId) ;
```
As shown, this function accepts following 7 parameters,
* *ManagedDevice struct Instance* which has all the device info filled
* *client* reference to the ManagedDevice
* *Latitude* in decimal degrees using WGS84
* *Longitude* in decimal degrees using WGS84
* *elevation*	Elevation in meters using WGS84
* *measuredDateTime* When the location information is retrieved in string in ISO8601 format
* *accuracy*	Accuracy of the position in meters
* *request ID* out value of Request Id for the current request.

Refer to the documentation <a href="https://docs.internetofthings.ibmcloud.com/devices/device_mgmt/index.html#/update-location#update-location">documentation</a> for more information about the Location update.

Append/Clear Error Codes
-----------------------------------------------

Devices can choose to notify the IBM Watson Internet of Things Platform about changes in their error status. The Device can invoke  addErrorCode() function to add the current error code to Watson IoT Platform.

``` {.sourceCode .c}
	addErrorCode(&client, 121 , reqId);
```
As shown, this function accepts following 3 parameters,
* *ManagedDevice struct Instance* which has all the device info filled.
* *ErrorCode* error code to be added as integer.
* *request ID* out value of Request Id for the current request.
Also, the ErrorCodes can be cleared from IBM Watson Internet of Things Platform by calling the clearErrorCodes() function as follows:

``` {.sourceCode .c}
	clearErrorCodes(&client,  reqId);
```
As shown, this function accepts following 2 parameters,
* *ManagedDevice struct Instance* which has all the device info filled
* *request ID* out value of Request Id for the current request.

Append/Clear Log messages
-----------------------------
Devices can choose to notify the IBM Watson Internet of Things Platform about changes by adding a new log entry. Log entry includes a log messages, its time stamp and severity, as well as an optional base64-encoded binary diagnostic data. The Devices can invoke addLog() function to send log messages,

``` {.sourceCode .c}
addLog(&client, "test","",1, reqId);
```
As shown, this function accepts following 2 parameters,
* *ManagedDevice struct Instance* which has all the device info filled.
* *message* log information.
* *data* optional base64-encoded binary diagnostic data as string.
* *request ID* out value of Request Id for the current request.


Also, the log messages can be cleared from IBM Watson Internet of Things Platform by calling the clearLogs() function as follows:

``` {.sourceCode .c}
clearLogs(&client,reqId);
```
As shown, this function accepts following 2 parameters,
* *ManagedDevice struct Instance* which has all the device info filled
* *request ID* out value of Request Id for the current request.

The device diagnostics operations are intended to provide information on device errors, and does not provide diagnostic information relating to the devices connection to the IBM Watson Internet of Things Platform.

Refer to the documentation <a href="https://docs.internetofthings.ibmcloud.com/devices/device_mgmt/index.html#/update-location#update-location">documentation</a> for more information about the Diagnostics operation.

Handling commands
-----------------

When the device client connects, it automatically subscribes to any
command for this device. To process specific commands you need to
register a command callback function by calling the function
*setCommandHandler*. The commands are returned as

-   commandName - name of the command invoked
-   format - e.g json, xml
-   payload

``` {.sourceCode .c}
#include "devicemanagementclient.h"

void myCallback (char* commandName, char* format, void* payload)
{
   printf("The command received :: %s\n", commandName);
   printf("format : %s\n", format);
   printf("Payload is : %s\n", (char *)payload);
}
 ...
 ...
 char *filePath = "./device.cfg";
 rc = connectiotfConfig_dm(filePath);
 setCommandHandler_dm(myCallback);

 yield_dm(100);
 ....
```

**Note** : *yield_dm* function must be called periodically to receive commands.



Disconnect Client
------------------

Disconnects the client and releases the connections

``` {.sourceCode .c}
  disconnect_dm(&client);
```
As shown, this function accepts following parameter,
* *ManagedDevice struct Instance* which has all the device info filled
