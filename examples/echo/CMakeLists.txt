project(ExamplesEcho)

# Configuration

#    Generate config files
#set(TEST_DATA_FILE ${CMAKE_CURRENT_SOURCE_DIR}/test_data/jpg.jpg)

#    Find all source files

include_directories(
    ${Evnet_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(echo_server echo_server.cc)
target_link_libraries(echo_server evnet glog pthread)

add_executable(echo_client echo_client.cc)
target_link_libraries(echo_client evnet glog pthread)

add_executable(chargen_client chargen_client.cc)
target_link_libraries(chargen_client evnet glog pthread)
