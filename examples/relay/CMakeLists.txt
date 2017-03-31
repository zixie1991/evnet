project(relay)

# Configuration

#    Generate config files
#set(TEST_DATA_FILE ${CMAKE_CURRENT_SOURCE_DIR}/test_data/jpg.jpg)

#    Find all source files

include_directories(
    ${Evnet_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(relay_server relay_server.cc tunnel.cc)
target_link_libraries(relay_server evnet glog pthread)
