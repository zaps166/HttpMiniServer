cmake_minimum_required(VERSION 3.1)

project(HttpMiniServer)

# set default build type as Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 11)

find_package(Qt5Core)
find_package(Qt5Network)

set(SOURCE_FILES
    "main.cpp"
    "HttpServer.cpp"
    "HttpClient.cpp"
)

set(HEADER_FILES
    "HttpServer.hpp"
    "HttpClient.hpp"
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang") # GCC or Clang
    option(USE_LINK_TIME_OPTIMIZATION "Enable link time optimization for release builds" OFF)

    set(CMAKE_CXX_FLAGS_RELEASE    "-fno-exceptions ${CMAKE_CXX_FLAGS_RELEASE}")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "-fno-exceptions ${CMAKE_CXX_FLAGS_MINSIZEREL}")

    if(USE_LINK_TIME_OPTIMIZATION)
        set(CMAKE_CXX_FLAGS_RELEASE    "${CMAKE_CXX_FLAGS_RELEASE} -flto")
        set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_RELEASE} -flto")
    endif()

    if(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)
        string(REPLACE "-rdynamic" "" CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS ${CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS})
    endif()
endif()

add_executable(${PROJECT_NAME}
    ${SOURCE_FILES}
    ${HEADER_FILES}
)

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Network
)

#Installation
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
