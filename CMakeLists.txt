cmake_minimum_required(VERSION 2.8.8)
project(Network)

#    Install Prefix
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Default install path" FORCE )
endif()

###    Configuration    ###########################################################################
#    Compiler Flags
set(CMAKE_CXX_COMPILER_FLAGS "${CMAKE_CXX_COMPILER_FLAGS} -Wall")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -O2 -pipe -Wall -Wextra") # set global flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -pipe -Wall -Wextra") # set global flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -ggdb -pg") # set debug flags
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s -DNDEBUG") # set release flags

# Include Directories
set(${PROJECT_NAME}_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include)
include_directories(${${PROJECT_NAME}_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/src)

###    Subdirectories    ##########################################################################
add_subdirectory(src/net)
add_subdirectory(src/util)
# add_subdirectory(src/test)
# add_subdirectory(tools)
# add_subdirectory(examples/echo)
# add_subdirectory(examples/relay)
