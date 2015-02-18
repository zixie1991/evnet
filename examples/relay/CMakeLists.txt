project(relay)

# Configuration

#    Generate config files
#set(TEST_DATA_FILE ${CMAKE_CURRENT_SOURCE_DIR}/test_data/jpg.jpg)

#    Find all source files

include_directories(
    ${Network_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(relayserver relayserver.cpp tunnel.cpp)
target_link_libraries(relayserver network pthread)

#    Build each source file independently
#foreach(source ${TOOLS_SOURCES})
    #get_filename_component(name ${source} NAME_WE)
    #add_executable(${name}.bin ${source})
    #set_target_properties(${name}.bin PROPERTIES OUTPUT_NAME ${name})
    #target_link_libraries(${name}.bin imagetool gtest pthread)

###    Install    #################################################################################

    #install(TARGETS ${name}.bin DESTINATION tools)

#endforeach(source)

