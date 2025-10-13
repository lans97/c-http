### C HTTP Server Project
cmake_minimum_required(VERSION 3.15)

set(CMAKE_C_STANDARD 23)

project(
    http_server
        VERSION 1.0
        LANGUAGES C
)

file(
    GLOB_RECURSE LIB_SOURCES
        "src/*.c" "src/**/*.c"
)

add_library(
    http SHARED
        ${LIB_SOURCES}
)

### Dependencies
include(FetchContent)

FetchContent_Declare(
    logger
    GIT_REPOSITORY  http://github.com/lans97/c-logger.git
    GIT_TAG         main
)

FetchContent_MakeAvailable(logger)

FetchContent_Declare(
    sds
    GIT_REPOSITORY  http://github.com/lans97/sds.git
    GIT_TAG         master
)

FetchContent_MakeAvailable(sds)

set_target_properties(sds PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

target_link_libraries(
    http PRIVATE
        sds::sds
        logger
)

target_compile_features(
    http PUBLIC
    c_std_23
)

target_include_directories(
    http PRIVATE
        "src/"
        "include/"
)

### Testing

enable_testing()

# C Unity Test Project

FetchContent_Declare(
    unity
    GIT_REPOSITORY  http://github.com/ThrowTheSwitch/Unity
    GIT_TAG         v2.6.1
)

FetchContent_MakeAvailable(unity)

# Executables
add_executable( map_test "test/map_test.c" ${LIB_SOURCES})
add_executable( request_test "test/request_test.c" ${LIB_SOURCES})
add_executable( response_test "test/response_test.c" ${LIB_SOURCES})

# Linking
target_link_libraries( map_test PRIVATE http sds::sds logger unity)
target_link_libraries( request_test PRIVATE http sds::sds logger unity)
target_link_libraries( response_test PRIVATE http sds::sds logger unity)

# Include
target_include_directories( map_test PRIVATE "src/" "include/")
target_include_directories( request_test PRIVATE "src/" "include/")
target_include_directories( response_test PRIVATE "src/" "include/")

# Test register
add_test( NAME map COMMAND map_test)
add_test( NAME request COMMAND request_test)
add_test( NAME response COMMAND response_test)
