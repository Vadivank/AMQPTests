cmake_minimum_required(VERSION 3.1)
set(PROJECT_NAME HelloAMQP)
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(${PROJECT_NAME} hello.cpp AMQPHandler.cpp AMQPHandler.h)

target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS})
