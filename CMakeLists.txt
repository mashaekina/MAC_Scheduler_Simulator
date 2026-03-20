cmake_minimum_required(VERSION 3.16)
project(telecom_scheduler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable nice output for running tests
include(CTest)

add_subdirectory(src)
add_subdirectory(tests)
