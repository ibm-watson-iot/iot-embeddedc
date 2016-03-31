cmake_minimum_required(VERSION 2.8)

add_executable (helloWorld helloWorld.c)
add_executable (sampleDevice sampleDevice.c)
add_executable (sampleDeviceManagement sampleDeviceManagement.c)
add_executable (sampleGateway sampleGateway.c)

target_link_libraries (helloWorld wiotdevice)
target_link_libraries (sampleDevice wiotdevice)
target_link_libraries (sampleDeviceManagement iotfdevmgt wiotdevice)
target_link_libraries (sampleGateway iotf)

include_directories(../ ../lib)
