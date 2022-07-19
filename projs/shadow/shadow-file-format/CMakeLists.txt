set(CMAKE_CXX_STANDARD 20)

# Set up Catch2 testing
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
enable_testing()

# Set up asset sourceset
FILE(GLOB_RECURSE SOURCES src/*.cpp src/*.h)
FILE(GLOB_RECURSE TESTS test/*.cpp)

add_library(shadow-asset ${SOURCES})

# Set up test executable
add_executable(shadow-asset-test ${TESTS})
target_link_libraries(shadow-asset-test PRIVATE Catch2::Catch2 shadow-utils)

# Enable testing on the executable
include(CTest)
include(Catch)
catch_discover_tests(shadow-asset-test)