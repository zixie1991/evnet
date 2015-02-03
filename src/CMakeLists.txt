project(NetworkSrc)

#    OpenCV
#find_package(OpenCV REQUIRED)

#    Recursively find source files
##    test sources
file(GLOB_RECURSE TEST_CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*_test.cpp)

##    all cpp sources
file(GLOB_RECURSE CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(network ${CPP_SOURCES})


#set output directory
set_target_properties(network PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

add_subdirectory(test)
