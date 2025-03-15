cmake_minimum_required(VERSION 3.16)

project(
    R2F-MessengerServer
    VERSION  2.0
    LANGUAGES CXX
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
    message(STATUS "Configuring for FreeBSD")
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    message(STATUS "Configuring for Linux")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

find_package(Qt5 COMPONENTS Core Network Sql REQUIRED)

find_path(GMP_INCLUDE_DIR NAMES gmp.h
  PATHS /usr/local/include /usr/include
)

find_library(GMP_LIBRARY NAMES gmp
  PATHS /usr/local/lib /usr/lib
)

if (GMP_INCLUDE_DIR AND GMP_LIBRARY)
  message(STATUS "GMP found")
else()
  message(FATAL_ERROR "GMP not found")
endif()

add_executable(R2F-MessengerServer
    include/BackEnd.h
    include/ConnectedClientManager.h
    include/Database.h
    include/Logger.h
    include/MessageProcessor.h
    include/ServerManager.h
    include/SessionKey.h
    include/server.h
    include/stdafx.h
    include/messagecode.h
    include/TcpDumpManager.h
    include/PacketAnalyzer.h

    src/BackEnd.cpp
    src/ConnectedClientManager.cpp
    src/Database.cpp
    src/MessageProcessor.cpp
    src/ServerManager.cpp
    src/SessionKey.cpp
    src/main.cpp
    src/server.cpp
    src/TcpDumpManager.cpp
    src/PacketAnalyzer.cpp

    ${QT_RESOURCES}
)

target_link_options(R2F-MessengerServer PRIVATE -v)

target_link_libraries(R2F-MessengerServer PRIVATE
    Qt5::Core
    Qt5::Network
    Qt5::Sql
)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    target_compile_options(R2F-MessengerServer PRIVATE
        -fsanitize=address
        -Wthread-safety
    )
    target_link_options(R2F-MessengerServer PRIVATE
        -fsanitize=address
        -Wthread-safety
    )
endif()

target_include_directories(R2F-MessengerServer PRIVATE ${GMP_INCLUDE_DIR})
target_link_libraries(R2F-MessengerServer PRIVATE
   ${GMP_LIBRARY}
)

target_link_libraries(R2F-MessengerServer PRIVATE ${CMAKE_THREAD_LIBS_INIT})

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/../build")
install(TARGETS R2F-MessengerServer)

