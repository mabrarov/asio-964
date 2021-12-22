cmake_minimum_required(VERSION 3.0)
project(asio-964)

find_package(Boost 1.63.0 REQUIRED COMPONENTS regex date_time chrono system)

if(NOT Boost_USE_STATIC_LIBS)
    list(APPEND compile_definitions "BOOST_ALL_DYN_LINK")
endif()
if(WIN32)
    list(APPEND compile_definitions
        WIN32
        WIN32_LEAN_AND_MEAN
        WINVER=0x0501
        _WIN32_WINNT=0x0501
        _WIN32_WINDOWS=0x0501
        _WIN32_IE=0x0600
        _UNICODE
        UNICODE
        _WINSOCK_DEPRECATED_NO_WARNINGS)
endif()

if (ASIO_964_RAW)
    list(APPEND compile_definitions RAW)
endif()

set(include_directories ${Boost_INCLUDE_DIRS})
set(libraries ${Boost_LIBRARIES})

if(UNIX AND NOT APPLE)
    list(APPEND libraries "rt")
endif()
if(WIN32)
    list(APPEND libraries "ws2_32" "mswsock")
endif()

add_executable(asio-964 main.cpp)
target_compile_definitions(asio-964 PRIVATE ${compile_definitions})
target_include_directories(asio-964 PRIVATE ${include_directories})
target_link_libraries(asio-964 PRIVATE ${libraries})
