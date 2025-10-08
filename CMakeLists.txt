### C HTTP Server Project
cmake_minimum_required(VERSION 3.15)

set(CMAKE_C_STANDARD 23)

project(
    http_server
    VERSION 1.0
    LANGUAGES C
)

add_library(http SHARED
    src/http.c
    #src/**/*.c
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
