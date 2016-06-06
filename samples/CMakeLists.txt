cmake_minimum_required(VERSION 2.8)
include_directories ("${PROJECT_SOURCE_DIR}/src")
include_directories ("${PROJECT_SOURCE_DIR}/lib")

#add_library(testUtils testUtils.c)

add_executable(helloWorld helloWorld.c)

add_executable(sampleDevice sampleDevice.c)
#add_test(test_devicemgmt test_devicemgmt)

add_executable(sampleDeviceManagement sampleDeviceManagement.c)
add_executable(sampleDMActions sampleDMActions.c)
add_executable(sampleDMFirmwareActions sampleDMFirmwareActions.c)
add_executable(sampleGateway sampleGateway.c)
#set(LINK_LIBS "testUtils iotfclient libiotfdevmgt cmocka m")
target_link_libraries(helloWorld iotfclient )
target_link_libraries(sampleDevice iotfclient )
target_link_libraries(sampleDeviceManagement  iotfdevmgt m)
target_link_libraries(sampleDMActions iotfclient iotfdevmgt m)
target_link_libraries(sampleDMFirmwareActions iotfdevmgt m)
target_link_libraries(sampleGateway  iotfgwayclient)
