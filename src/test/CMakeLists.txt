project(Test)

# Configuration
set(TEST_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test)    # test executables are going to be placed there
set(TEST_EXT .testbin)           # test executable extension
set(ALL_TEST test${TEST_EXT})    # name of an executable comprising of all tests
set(RUN_TEST runtest)            # dummy target for running tests
set(TEST_MAIN network_test_main.cpp)     # main test file (with main function)

#    Generate config files
#set(TEST_DATA_FILE ${CMAKE_CURRENT_SOURCE_DIR}/test_data/jpg.jpg)

#    Find all source files
file(GLOB_RECURSE TOOLS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*_test.cpp)

include_directories(
    ${Network_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(network_test_main ${TOOLS_SOURCES})
target_link_libraries(network_test_main network gtest pthread)

add_executable(eventloop_test eventloop_test_.cpp)
target_link_libraries(eventloop_test network pthread)

add_executable(acceptor_test acceptor_test_.cpp)
target_link_libraries(acceptor_test network pthread)

add_executable(connection_test connection_test_.cpp)
target_link_libraries(connection_test network pthread)

#    Build each source file independently
#foreach(source ${TOOLS_SOURCES})
    #get_filename_component(name ${source} NAME_WE)
    #add_executable(${name}.bin ${source})
    #set_target_properties(${name}.bin PROPERTIES OUTPUT_NAME ${name})
    #target_link_libraries(${name}.bin imagetool gtest pthread)

###    Install    #################################################################################

    #install(TARGETS ${name}.bin DESTINATION tools)

#endforeach(source)

