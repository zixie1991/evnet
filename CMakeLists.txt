cmake_minimum_required(VERSION 2.8.8)
project(Evnet)

set(CMAKE_VERBOSE_MAKEFILE ON)

#    Install Prefix
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Default install path" FORCE )
endif()

###    Configuration    ###########################################################################
#    Compiler Flags
set(CMAKE_CXX_COMPILER_FLAGS "${CMAKE_CXX_COMPILER_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC -pipe -Wall -Wextra") # set global flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -ggdb -pg") # set debug flags
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -mavx -s -DNDEBUG") # set release flags

# third party
set(PROJECT_THIRD_PARTY_INCLUDE ${PROJECT_SOURCE_DIR}/third_party/include)
set(PROJECT_THIRD_PARTY_LIB ${PROJECT_SOURCE_DIR}/third_party/lib)
# Include Directories
set(${PROJECT_NAME}_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include)
include_directories(${${PROJECT_NAME}_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/src)
link_directories(${CMAKE_SOURCE_DIR}/third_party/lib)
set(evnet_LINKER_LIBS "")
# list(APPEND net_LINKER_LIBS ${mshadow_LINKER_LIBS})

###    Subdirectories    ##########################################################################
add_subdirectory(src/evnet)
# add_subdirectory(src/util)
add_subdirectory(src/test)
# add_subdirectory(tools)
add_subdirectory(examples/echo)
add_subdirectory(examples/relay)
add_subdirectory(examples/http_relay)
