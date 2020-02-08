cmake_minimum_required(VERSION 3.5)

project(message_bus LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " -pthread")

add_executable(message_bus
    main.cpp
    message_bus.h)
