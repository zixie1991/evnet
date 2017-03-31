project(Evnet)
set(evnet_LINKER_LIBS "")
#    glog
include_directories(${GLOG_INCLUDE_DIRS})
list(APPEND evnet_LINKER_LIBS glog)
list(APPEND evnet_LINKER_LIBS boost_system)
#    Recursively find source files
##    all cpp sources
file(GLOB_RECURSE CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)
# file(GLOB_RECURSE CPP_TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*_test.cc)
#list(REMOVE_ITEM CPP_SOURCES ${CPP_TEST_SOURCES})
list(REMOVE_ITEM CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*_test.cc)
#add_library(libcommon SHARED ${CPP_SOURCES})
add_library(evnet STATIC ${CPP_SOURCES})
target_link_libraries(evnet ${evnet_LINKER_LIBS})
# target_link_libraries(libcommon ${common_LINKER_LIBS})
# set_target_properties(libcommon PROPERTIES OUTPUT_NAME "common")
#set output directory
#set_target_properties(common libcommon PROPERTIES
        #ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        #LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
#)
set_target_properties(evnet PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)
