cmake_minimum_required(VERSION 2.8)

project(iotf-embeddedc)
set(CMAKE_BUILD_TYPE Release)

set(CMAKE_INSTALL_PREFIX "/usr")

#
# Samples
#
option (BUILDSAMPLES "Build sample applications." OFF)

if (BUILDSAMPLES)
  add_subdirectory (samples)
endif ()

#
# Core Libraries
#
 
# Headers
include_directories(. lib)

file(GLOB LIBS "lib/*.c")

set (LIB_GLOB_HEADERS
  devicemanagementclient.h
  gatewayclient.h
  iotfclient.h
)
 
# Generate shared libraries
add_library(iotfdevmgt SHARED devicemanagementclient.c ${LIBS})
add_library(iotf SHARED gatewayclient.c ${LIBS})
add_library(wiotdevice SHARED iotfclient.c ${LIBS})

# Install header files
install (FILES ${LIB_GLOB_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
 
# Install libraries
install(TARGETS iotfdevmgt DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(TARGETS wiotdevice DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(TARGETS iotf DESTINATION ${CMAKE_INSTALL_LIBDIR})
