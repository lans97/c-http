cmake_minimum_required(VERSION 3.15)

set(CMAKE_C_STANDARD 23)

project(
    http
    VERSION 1.0
    LANGUAGES C)

file(GLOB_RECURSE sources src/*.c src/*.h)

file(GLOB_RECURSE sds_sources
        "vendor/sds/*.c"
        "vendor/sds/*.h")

add_executable(main ${sources} ${sds_sources})

find_package(mimalloc 2.1 REQUIRED)

target_link_libraries(
    main PUBLIC
    mimalloc
)

target_compile_features(
    main PRIVATE
    c_std_23
)

target_include_directories(
    main PUBLIC
    "src/"
    "vendor/sds/"
)
