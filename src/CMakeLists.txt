cmake_minimum_required(VERSION 2.8)
include_directories ("${PROJECT_SOURCE_DIR}/lib")
add_library(iotfdevmgt SHARED devicemanagementclient.c ${PROJECT_SOURCE_DIR}/lib/cJSON.c)
add_library(iotfgwayclient SHARED gatewayclient.c)
add_library(iotfclient SHARED iotfclient.c)

TARGET_LINK_LIBRARIES (iotfdevmgt iotfclient iotfmqttclient)
TARGET_LINK_LIBRARIES (iotfgwayclient iotfmqttclient)
TARGET_LINK_LIBRARIES (iotfclient iotfmqttclient)
