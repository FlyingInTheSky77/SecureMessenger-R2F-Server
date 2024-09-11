cmake_minimum_required(VERSION 3.16)

project(
    R2F-MessengerServer
    VERSION  2.0
    LANGUAGES CXX
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

find_package(Qt5 COMPONENTS Core Network Sql REQUIRED)

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

target_link_libraries(R2F-MessengerServer PRIVATE
    gmp
)

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/../build")
install(TARGETS R2F-MessengerServer)

