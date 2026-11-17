cmake_minimum_required(VERSION 3.15)
project(SmartCampusNavigation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

include(FetchContent)

# ------------------------------------------------------------------
# Crow (header-only C++ web framework) and standalone Asio (networking).
# Both are free and are downloaded automatically from GitHub on the
# first cmake run. If you already installed them system-wide
# (apt / brew / vcpkg), those copies are used instead.
# ------------------------------------------------------------------
find_package(Crow QUIET)

if(NOT Crow_FOUND)
  find_package(asio QUIET)
  find_path(ASIO_SYSTEM_INCLUDE asio.hpp)

  if(NOT asio_FOUND AND NOT ASIO_SYSTEM_INCLUDE)
    FetchContent_Declare(
      asio
      GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
      GIT_TAG        asio-1-30-2
      GIT_SHALLOW    TRUE
    )
    FetchContent_GetProperties(asio)
    if(NOT asio_POPULATED)
      FetchContent_Populate(asio)
    endif()
    set(ASIO_INCLUDE_DIR "${asio_SOURCE_DIR}/asio/include" CACHE PATH "" FORCE)
  endif()

  FetchContent_Declare(
    Crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG        v1.2.0
    GIT_SHALLOW    TRUE
  )
  set(CROW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(CROW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(CROW_INSTALL OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(Crow)
endif()

add_executable(campus_server main.cpp)
target_link_libraries(campus_server PRIVATE Crow::Crow)

if(ASIO_INCLUDE_DIR)
  target_include_directories(campus_server PRIVATE ${ASIO_INCLUDE_DIR})
endif()

find_package(Threads REQUIRED)
target_link_libraries(campus_server PRIVATE Threads::Threads)

if(WIN32)
  target_link_libraries(campus_server PRIVATE ws2_32 mswsock)
endif()
