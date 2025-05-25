cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 11)
project(Webserver CXX)


include_directories(${PROJECT_SOURCE_DIR}/include)
add_executable(server src/poll_server.cpp src/User.cpp)
add_executable(client src/poll_client.cpp src/User.cpp)