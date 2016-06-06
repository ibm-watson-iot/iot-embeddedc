cmake_minimum_required(VERSION 2.8)
project(iot-embeddedc)
enable_testing()

#SET(CMAKE_CXX_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage -fPIC ")
#SET(CMAKE_C_FLAGS "-g -O0 -Wall -W -fprofile-arcs -ftest-coverage -fPIC ")
#SET(CMAKE_EXE_LINKER_FLAGS "-fprofile-arcs -ftest-coverage ")
option (run_tests "set run_tests to ON if build tests should be run, set to OFF to skip tests" ON)
SET(CMAKE_CXX_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage -fPIC ${CMAKE_CXX_FLAGS} ")
SET(CMAKE_C_FLAGS "-g -O0 -Wall -W -fprofile-arcs -ftest-coverage -fPIC ${CMAKE_C_FLAGS} ")
SET(CMAKE_EXE_LINKER_FLAGS "-fprofile-arcs -ftest-coverage ${CMAKE_EXE_LINKER_FLAGS} ")

if (run_tests)
    add_subdirectory(test)
endif ()

add_subdirectory(src)
add_subdirectory(lib)
add_subdirectory(samples)

