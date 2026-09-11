cmake_minimum_required(VERSION 3.22.1)

project("mod_installer")

# Minizip-ng
set(MZ_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(MZ_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(MZ_BUILD_FUZZ_TESTS OFF CACHE BOOL "" FORCE)

set(MZ_BZIP2 OFF CACHE BOOL "" FORCE)
set(MZ_LZMA OFF CACHE BOOL "" FORCE)
set(MZ_PPMD OFF CACHE BOOL "" FORCE)
set(MZ_ZSTD OFF CACHE BOOL "" FORCE)

set(MZ_ZLIB ON CACHE BOOL "" FORCE)
set(MZ_COMPAT OFF CACHE BOOL "" FORCE)

add_subdirectory(minizip-ng)

# Nossa biblioteca
add_library(${CMAKE_PROJECT_NAME} SHARED
        native-lib.cpp
        app.cpp
        paths.cpp
        utils.cpp
        modData.cpp
        modInstallData.cpp
        console.cpp
        package.cpp
        mod_database.cpp
        download.cpp
        zip.cpp
)

# Curl via Prefab
find_package(curl REQUIRED CONFIG)

target_link_libraries(${CMAKE_PROJECT_NAME}
        android
        log
        curl::curl_static
        MINIZIP::minizip
)