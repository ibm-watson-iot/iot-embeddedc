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
