cmake_minimum_required(VERSION 3.14)
project(TaskflowCLI CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler-specific warning flags (MSVC does not understand -Wall/-Wextra)
if(MSVC)
    add_compile_options(/W4 /utf-8)
else()
    add_compile_options(-Wall -Wextra)
endif()

# Core library (everything except main.cpp) so the test binary can link
# against the same logic the CLI uses.
add_library(taskflow_core
    TaskManager.cpp
    Display.cpp
)
target_include_directories(taskflow_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# CLI executable
add_executable(taskflow main.cpp)
target_link_libraries(taskflow PRIVATE taskflow_core)

# Tests
enable_testing()
add_executable(taskflow_tests tests/test_task_manager.cpp)
target_link_libraries(taskflow_tests PRIVATE taskflow_core)
add_test(NAME taskflow_tests COMMAND taskflow_tests)
