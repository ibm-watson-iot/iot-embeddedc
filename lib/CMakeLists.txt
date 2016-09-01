cmake_minimum_required(VERSION 2.8)
add_library(iotfmqttclient MQTTClient.c MQTTLinux.c MQTTFormat.c MQTTPacket.c MQTTDeserializePublish.c MQTTDeserializePublish.c MQTTConnectClient.c MQTTSubscribeClient.c MQTTSerializePublish.c MQTTConnectServer.c MQTTSubscribeServer.c MQTTUnsubscribeServer.c MQTTUnsubscribeClient.c)
