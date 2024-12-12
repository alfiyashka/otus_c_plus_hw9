cmake_minimum_required(VERSION 3.28.3)

set(PATCH_VERSION "1" CACHE INTERNAL "PATCH_VERSION" )
set(PROJECT_VERSION 0.0.${PATCH_VERSION})



project(async VERSION ${PROJECT_VERSION} LANGUAGES C CXX)

configure_file(version.h.in version.h)

set ( HW9_INCLUDE
   include/cmd.parse/CmdFileWriter.hpp
   include/cmd.parse/CommandParser.hpp
   include/cmd.parse/CmdParserState.hpp
   include/async/async.h
   include/async/TaskManager.hpp
)

include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inlude)

set ( HW9_SOURCES
   cmd.parse/CmdFileWriter.cpp
   cmd.parse/CommandParser.cpp
   cmd.parse/CmdParserState.cpp
   async/async.cpp
   async/TaskManager.cpp
)

find_package (Threads)

add_library(async SHARED ${HW9_SOURCES})


target_link_libraries (async Threads::Threads)
set_property(TARGET async PROPERTY CXX_STANDARD 20)

install(TARGETS async DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/async_user/asynclib/lib)

install(DIRECTORY include DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/async_user/asynclib/include)


include("functions.cmake")
generate_packages(${PROJECT_NAME})


#async_user
project(async_user LANGUAGES C CXX)


include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/asynclib/include/include)


add_executable(async_user async_user/main.cpp)

add_library(asynclib SHARED IMPORTED)
set_property(TARGET asynclib PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/async_user/asynclib/lib/libasync.so)

target_link_libraries (async_user asynclib)

set_property(TARGET async_user PROPERTY CXX_STANDARD 20)


include("functions.cmake")
