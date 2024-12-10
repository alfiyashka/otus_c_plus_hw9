cmake_minimum_required(VERSION 3.28.3)


set(PATCH_VERSION "1" CACHE INTERNAL "PATCH_VERSION" )
set(PROJECT_VERSION 0.0.${PATCH_VERSION})


project(async_user VERSION ${PROJECT_VERSION} LANGUAGES C CXX)


configure_file(version.h.in version.h)

include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/asynclib/include/include)


add_executable(async_user main.cpp)

add_library(asynclib SHARED IMPORTED)
set_property(TARGET asynclib PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/asynclib/lib/libasync.so)

target_link_libraries (async_user asynclib)

set_property(TARGET async_user PROPERTY CXX_STANDARD 20)


include("functions.cmake")
generate_packages(${PROJECT_NAME})

