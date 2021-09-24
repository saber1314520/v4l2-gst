#basic options
cmake_minimum_required(VERSION 3.0.0)

project(MAIN)

#compile command options
add_compile_options(-Wall -g -std=c++11 -o2)

SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-pthread")

# SET(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-pthread")

find_package(PkgConfig)
pkg_search_module(GSTREAMER REQUIRED gstreamer-1.0)
MESSAGE(STATUS "gst dirs:" ${GSTREAMER_INCLUDE_DIRS})
MESSAGE(STATUS "gst libs:" ${GSTREAMER_LIBRARIES})
include_directories(${GSTREAMER_INCLUDE_DIRS})
link_directories(${GSTREAMER_LIBRARIES})

SET(ENV(PKG_CONFIG_PATH) )

#file path options
include_directories(include)

#aux_source_directory()

#add_subdirectory()

# add_library (testFunc_shared SHARED ${SRC_LIST})
# add_library (testFunc_static STATIC ${SRC_LIST})
# set_target_properties (testFunc_shared PROPERTIES OUTPUT_NAME "testFunc")
# set_target_properties (testFunc_static PROPERTIES OUTPUT_NAME "testFunc")
# set (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

#find_library()
set(SRC_LIST 
    ./src/gpptest.cpp
    ./src/v4l2IOctrl.cpp
    ./src/timestamp.cpp
    ./src/log.cpp)

add_executable(gpptest ${SRC_LIST})

target_link_libraries(gpptest ${GSTREAMER_LIBRARIES})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)