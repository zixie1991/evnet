project(EvnetTest)
# Configuration
set(TEST_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test)    # test executables are going to be placed there
set(TEST_MAIN test_main.cc)     # main test file (with main function)
##    test sources
file(GLOB_RECURSE TEST_CPP_SOURCES ${CMAKE_SOURCE_DIR}/*_test.cc)
#    Find all source files
include_directories(
    ${FaceRec_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)
add_executable(test_main test_main.cc ${TEST_CPP_SOURCES})
target_link_libraries(test_main gtest pthread)
