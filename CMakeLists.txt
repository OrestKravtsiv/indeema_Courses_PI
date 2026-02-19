cmake_minimum_required(VERSION 3.10)
project(coreapp)
set(CMAKE_CXX_STANDARD 20)

find_package(PkgConfig REQUIRED)
find_package(jsoncpp REQUIRED)
pkg_check_modules(SERIAL REQUIRED libserial)

add_subdirectory(common)

add_executable(coreapp main.cpp)
target_link_libraries(coreapp PRIVATE common)