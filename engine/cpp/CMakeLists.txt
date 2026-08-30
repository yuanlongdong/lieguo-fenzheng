cmake_minimum_required(VERSION 3.14)
project(lieguo_fenzheng LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
# 核心引擎库
add_library(lieguo_engine STATIC
    src/config.cpp
    src/game.cpp
)
# 控制台 Demo
add_executable(lieguo_demo src/main.cpp)
target_link_libraries(lieguo_demo PRIVATE lieguo_engine)
# 线程支持
find_package(Threads REQUIRED)
target_link_libraries(lieguo_demo PRIVATE Threads::Threads)
# 单元测试（可选）
option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_executable(test_game tests/test_game.cpp)
    target_link_libraries(test_game PRIVATE lieguo_engine Threads::Threads)
    add_test(NAME game_test COMMAND test_game)
endif()
install(TARGETS lieguo_demo DESTINATION bin)
install(TARGETS lieguo_engine DESTINATION lib)
install(DIRECTORY include/ DESTINATION include/lieguo)
