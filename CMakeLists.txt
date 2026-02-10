cmake_minimum_required(VERSION 3.10)
project(coreapp)
set(CMAKE_CXX_STANDARD 20)
# 1. Look into the 'common' folder and run its CMakeLists.txt
add_subdirectory(common)
# 2. Define the app executable
add_executable(coreapp main.cpp)
# 3. Connect the library to the app
target_link_libraries(coreapp PRIVATE common)