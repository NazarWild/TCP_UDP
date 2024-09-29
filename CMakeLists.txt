cmake_minimum_required(VERSION 3.10)

project(MyApp)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(src main.cpp MessageContainer.cpp
        Message.cpp
        MessageContainerSingleton.cpp
        Connection.cpp
        CustomMessageContainer.cpp
)

add_executable(MyApp ${src})
